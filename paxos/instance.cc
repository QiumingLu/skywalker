// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/instance.h"

#include <stdio.h>

#include <utility>

#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

Instance::Instance(Config* config)
    : config_(config),
      acceptor_(config, this),
      learner_(config, this, &acceptor_),
      proposer_(config, this),
      instance_id_(0),
      is_proposing_(false),
      context_(nullptr) {}

Instance::~Instance() {}

bool Instance::Recover() {
  bool res = acceptor_.Recover(&instance_id_);
  if (!res) {
    LOG_ERROR("Group %u - Acceptor recover failed.", config_->GetGroupId());
    return res;
  }
  res = config_->GetLogManager()->Recover(&instance_id_);
  if (!res) {
    LOG_ERROR("Group %u - CheckpointManager recover failed.",
              config_->GetGroupId());
    return res;
  }
  acceptor_.SetInstanceId(instance_id_);
  learner_.SetInstanceId(instance_id_);
  proposer_.SetInstanceId(instance_id_);
  proposer_.SetStartProposalId(acceptor_.GetPromisedBallot().GetProposalId() +
                               1);

  LOG_INFO("Group %u - Instance recover successful, now instance_id=%llu.",
           config_->GetGroupId(), (unsigned long long)instance_id_);
  return res;
}

void Instance::SetIOLoop(RunLoop* loop) {
  io_loop_ = loop;
  proposer_.SetIOLoop(loop);
  learner_.SetIOLoop(loop);
}

void Instance::SetLearnLoop(RunLoop* loop) { learner_.SetLearnLoop(loop); }

void Instance::SyncData(bool add_timer) {
  io_loop_->QueueInLoop(
      [this, add_timer]() { learner_.AskForLearn(add_timer); });
}

void Instance::OnPropose(uint32_t machine_id, const std::string& value,
                         void* context) {
  if (!config_->IsValidNodeId(config_->GetNodeId())) {
    Slice msg("this node is not in the membership, please add it firstly.");
    propose_cb_(instance_id_, Status::InvalidNode(msg), context);
    return;
  }

  assert(!is_proposing_);
  is_proposing_ = true;

  assert(!context_);
  context_ = context;
  propose_value_.set_machine_id(machine_id);
  propose_value_.set_user_data(value);

  propose_timer_ = io_loop_->RunAfter(1000 * 1000, [this]() {
    proposer_.QuitPropose();
    is_proposing_ = false;
    Slice msg("proposal time more than a second.");
    propose_cb_(instance_id_, Status::Timeout(msg), context_);
    context_ = nullptr;
  });

  proposer_.NewPropose(propose_value_);
}

void Instance::OnContent(const Content& c) {
  switch (c.type()) {
    case PAXOS_MESSAGE:
      OnPaxosMessage(c.paxos_msg());
      break;
    case CHECKPOINT_MESSAGE:
      OnCheckpointMessage(c.checkpoint_msg());
      break;
    default:
      LOG_ERROR("Group %u - receive an invalid content.",
                config_->GetGroupId());
      break;
  }
}

void Instance::OnPaxosMessage(const PaxosMessage& msg) {
  if (learner_.IsReceivingCheckpoint()) {
    return;
  }
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
    case ASK_FOR_CHECKPOINT:
      learner_.OnAskForCheckpoint(msg);
      break;
    default:
      LOG_ERROR("Group %u - receive an invalid paxos message.",
                config_->GetGroupId());
      break;
  }

  CheckLearn();
}

void Instance::OnCheckpointMessage(const CheckpointMessage& msg) {
  learner_.OnSendCheckpoint(msg);
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
      Status status;
      if (success) {
        if (!my) {
          status = Status::Conflict("another value has been chosen.");
        }
      } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "machine(id=%u) execute failed.",
                 learned_value.machine_id());
        status = Status::MachineError(msg);
      }
      propose_cb_(instance_id_, status, context_);
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
  void* context = my ? context_ : nullptr;
  return config_->GetMachineManager()->Execute(instance_id_, value, context);
}

void Instance::NextInstance() {
  ++instance_id_;
  acceptor_.NextInstance();
  proposer_.NextInstance();
  learner_.NextInstance();
  LOG_INFO("Group %u - new instance is starting, which instance_id=%llu.",
           config_->GetGroupId(), (unsigned long long)instance_id_);
}

}  // namespace skywalker
