// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/instance.h"

#include <utility>

#include "paxos/config.h"
#include "util/runloop.h"
#include "util/mutexlock.h"
#include "skywalker/logging.h"

namespace skywalker {

Instance::Instance(Config* config)
    : config_(config),
      acceptor_(config, this),
      learner_(config, this, &acceptor_),
      proposer_(config, this),
      instance_id_(0),
      is_proposing_(false),
      context_(nullptr) {
}

Instance::~Instance() {
}

bool Instance::Init() {
  bool ret = acceptor_.Init(&instance_id_);
  if (!ret) {
    SWLog(ERROR, "Instance::Init - Acceptor init fail.\n");
    return ret;
  }
  acceptor_.SetInstanceId(instance_id_);
  learner_.SetInstanceId(instance_id_);
  proposer_.SetInstanceId(instance_id_);
  proposer_.SetStartProposalId(
      acceptor_.GetPromisedBallot().GetProposalId() + 1);

  SWLog(INFO,
        "Instance::Init - now instance_id=%" PRIu64".\n", instance_id_);
  return ret;
}

void Instance::SetIOLoop(RunLoop* loop) {
  io_loop_ = loop;
  proposer_.SetIOLoop(loop);
  learner_.SetIOLoop(loop);
}

void Instance::SetLearnLoop(RunLoop* loop) {
  learner_.SetLearnLoop(loop);
}

void Instance::SyncData() {
  io_loop_->QueueInLoop([this]() {
    learner_.AskForLearn();
  });
}

void Instance::AddMachine(StateMachine* machine) {
  MutexLock lock(&mutex_);
  machines_.insert(std::make_pair(machine->machine_id(), machine));
}

void Instance::RemoveMachine(StateMachine* machine) {
  MutexLock lock(&mutex_);
  machines_.erase(machine->machine_id());
}

void Instance::OnPropose(const std::string& value,
                         MachineContext* context) {
  if (!config_->IsValidNodeId(config_->GetNodeId())) {
    Slice msg("this node is not in the membership, please add it firstly.");
    propose_cb_(context, Status::InvalidNode(msg), instance_id_);
    return;
  }

  assert(!is_proposing_);
  is_proposing_ = true;

  assert(!context_);
  context_ = context;
  if (context_ != nullptr) {
    propose_value_.set_machine_id(context_->machine_id);
  } else {
    propose_value_.set_machine_id(-1);
  }
  propose_value_.set_user_data(value.data(), value.size());

  propose_timer_ = io_loop_->RunAfter(1000*1000, [this]() {
    proposer_.QuitPropose();
    is_proposing_ = false;
    Slice msg("proposal time more than a second.");
    propose_cb_(context_, Status::Timeout(msg), instance_id_);
  });

  proposer_.NewPropose(propose_value_);
}

void Instance::OnReceiveContent(const std::shared_ptr<Content>& c) {
  switch (c->type()) {
    case PAXOS_MESSAGE:
      OnPaxosMessage(c->paxos_msg());
      break;
    case CHECKPOINT_MESSAGE:
      OnCheckPointMessage(c->checkpoint_msg());
      break;
    default:
      SWLog(ERROR, "Instance::OnReceiveContent - Invalid content type.\n");
      break;
  }
}

void Instance::OnPaxosMessage(const PaxosMessage& msg) {
  switch (msg.type()) {
    case PREPARE:
      acceptor_.OnPrepare(msg);
      break;
    case ACCEPT:
      acceptor_.OnAccpet(msg);
      break;
    case PREPARE_REPLY:
      proposer_.OnPrepareReply(msg);
      break;
    case ACCEPT_REPLY:
      proposer_.OnAccpetReply(msg);
      break;
    case NEW_CHOSEN_VALUE:
      learner_.OnNewChosenValue(msg);
      break;
    case ASK_FOR_LEARN:
      learner_.OnAskForLearn(msg);
      break;
    case SEND_LEARNED_VALUE:
      learner_.OnSendLearnedValue(msg);
      break;
    case SEND_NOW_INSTANCE_ID:
      learner_.OnSendNowInstanceId(msg);
      break;
    case COMFIRM_ASK_FOR_LEARN:
      learner_.OnComfirmAskForLearn(msg);
      break;
    default:
      SWLog(ERROR, "Instance::OnPaxosMessage - Invalid message type.\n");
      break;
  }

  CheckLearn();
}

void Instance::OnCheckPointMessage(const CheckPointMessage& msg) {
  // TODO(handle checkpoint message)
}

void Instance::CheckLearn() {
  if (learner_.HasLearned()) {
    const PaxosValue& learned_value(learner_.GetLearnedValue());
    bool my = false;
    if (is_proposing_) {
      io_loop_->Remove(propose_timer_);
      if (propose_value_.machine_id() == learned_value.machine_id() &&
          propose_value_.user_data() == learned_value.user_data()) {
        my = true;
      }
    }

    bool success = MachineExecute(learned_value, my);

    if (is_proposing_) {
      if (success) {
       if (my) {
          propose_cb_(context_, Status::OK(), instance_id_);
        } else {
          Slice msg("another value has been chosen.");
          propose_cb_(context_, Status::Conflict(msg), instance_id_);
        }
      } else {
        Slice msg("machine execute failed.");
        propose_cb_(context_, Status::MachineError(msg), instance_id_);
      }
      is_proposing_ = false;
      context_ = nullptr;
    }

    if (success) {
      NextInstance();
    } else {
      proposer_.SetNoSkipPrepare();
    }
  }
}

bool Instance::MachineExecute(const PaxosValue& value, bool my) {
  int id = value.machine_id();
  if (id != -1) {
    MutexLock lock(&mutex_);
    auto i = machines_.find(id);
    if (i != machines_.end()) {
      assert(i->second != nullptr);
      MachineContext* context = nullptr;
      if (my) {
        context = context_;
      }
      return i->second->Execute(
          config_->GetGroupId(), instance_id_, value.user_data(), context);
    }
  }
  return true;
}

void Instance::NextInstance() {
  ++instance_id_;
  acceptor_.NextInstance();
  proposer_.NextInstance();
  learner_.NextInstance();
  SWLog(INFO, "Instance::NextInstance - new instance is starting, "
        "now instance_id=%" PRIu64".\n", instance_id_);
}

}  // namespace skywalker
