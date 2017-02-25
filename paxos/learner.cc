// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/learner.h"

#include <string>

#include "paxos/acceptor.h"
#include "paxos/config.h"
#include "paxos/instance.h"
#include "skywalker/logging.h"

namespace skywalker {

Learner::Learner(Config* config, Instance* instance, Acceptor* acceptor)
    : config_(config),
      messager_(config_->GetMessager()),
      instance_(instance),
      acceptor_(acceptor),
      instance_id_(0),
      max_instance_id_(0),
      max_instance_id_from_node_id_(0),
      rand_(static_cast<uint32_t>(NowMicros())),
      is_learning_(false),
      has_learned_(false) {
}

void Learner::SetInstanceId(uint64_t instance_id) {
  instance_id_ = instance_id;
}

void Learner::OnNewChosenValue(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    const BallotNumber& b = acceptor_->GetAcceptedBallot();
    BallotNumber ballot(msg.proposal_id(), msg.node_id());
    if (ballot == b) {
      FinishLearnValue(acceptor_->GetAcceptedValue());
      BroadcastMessageToFollower(b);
    } else if (msg.has_value()) {
      if (WriteToDB(msg)) {
        FinishLearnValue(msg.value());
        BroadcastMessageToFollower(b);
      }
    }
  }
  SetMaxInstanceId(msg.instance_id(), msg.node_id());
}

void Learner::AskForLearn() {
  is_learning_ = false;
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(ASK_FOR_LEARN);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  messager_->BroadcastMessage(messager_->PackMessage(msg));

  uint64_t delay = 1000 * (30 * 1000 + rand_.Uniform(10 * 1000));
  config_->GetLoop()->RunAfter(delay, [this]() {
    AskForLearn();
  });
}

void Learner::OnAskForLearn(const PaxosMessage& msg) {
  if (msg.instance_id() < instance_id_) {
    if (msg.instance_id() == instance_id_ - 1) {
      std::string s;
      int res = config_->GetDB()->Get(msg.instance_id(), &s);
      if (res == 0) {
        AcceptorState state;
        state.ParseFromString(s);
        SendLearnedValue(msg.node_id(), state);
      }
    } else {
      SendNowInstanceId(msg);
    }
  }
  SetMaxInstanceId(msg.instance_id(), msg.node_id());
}

void Learner::SendNowInstanceId(const PaxosMessage& msg) {
  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_type(SEND_NOW_INSTANCE_ID);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(msg.instance_id());
  reply_msg->set_now_instance_id(instance_id_);
  messager_->SendMessage(msg.node_id(), messager_->PackMessage(reply_msg));
}

void Learner::OnSendNowInstanceId(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_ &&
      msg.now_instance_id() > instance_id_) {
    if (!is_learning_) {
      ComfirmAskForLearn(msg);
      is_learning_ = true;
    }
  }
  SetMaxInstanceId(msg.now_instance_id(), msg.node_id());
}

void Learner::ComfirmAskForLearn(const PaxosMessage& msg) {
  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_type(COMFIRM_ASK_FOR_LEARN);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(instance_id_);
  messager_->SendMessage(msg.node_id(), messager_->PackMessage(reply_msg));
}

void Learner::OnComfirmAskForLearn(const PaxosMessage& msg) {
  uint64_t node_id = msg.node_id();
  uint64_t from = msg.instance_id();
  uint64_t to = instance_id_;
  config_->GetBGLoop()->QueueInLoop([node_id, from, to, this] {
    ASyncSend(node_id, from, to);
  });
}

void Learner::ASyncSend(uint64_t node_id, uint64_t from, uint64_t to) {
  while (from < to) {
    std::string s;
    int ret = config_->GetDB()->Get(from, &s);
    if (ret == 0) {
      AcceptorState state;
      state.ParseFromString(s);
      SendLearnedValue(node_id, state);
      ++from;
    } else {
      SWLog(ERROR, "Learner::OnComfirmAskForLearn - "
            "no found data of instance %" PRIu64"\n", from);
      break;
    }
  }
}

void Learner::SendLearnedValue(uint64_t node_id, const AcceptorState& state) {
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(state.instance_id());
  msg->set_proposal_id(state.accepted_id());
  msg->set_proposal_node_id(state.accepted_node_id());
  msg->set_allocated_value(new PaxosValue(state.accepted_value()));
  messager_->SendMessage(node_id, messager_->PackMessage(msg));
}

void Learner::OnSendLearnedValue(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    if (WriteToDB(msg)) {
      FinishLearnValue(msg.value());
      BallotNumber b(msg.proposal_id(), msg.node_id());
      BroadcastMessageToFollower(b);
    }
  }
}

void Learner::SetMaxInstanceId(uint64_t instance_id,
                               uint64_t node_id) {
  if (instance_id > max_instance_id_) {
    max_instance_id_ = instance_id;
    max_instance_id_from_node_id_ = node_id;
  }
}

bool Learner::WriteToDB(const PaxosMessage& msg) {
  AcceptorState state;
  state.set_instance_id(msg.instance_id());
  state.set_promised_id(msg.proposal_id());
  state.set_promised_node_id(msg.node_id());
  state.set_accepted_id(msg.proposal_id());
  state.set_accepted_node_id(msg.node_id());
  state.set_allocated_accepted_value(new PaxosValue(msg.value()));

  WriteOptions options;
  options.sync = false;

  std::string s;
  state.SerializeToString(&s);
  int res = config_->GetDB()->Put(options, msg.instance_id(), s);
  if (res == 0) {
    return true;
  } else {
    return false;
  }
}

void Learner::FinishLearnValue(const PaxosValue& value) {
  learned_value_ = value;
  has_learned_ = true;
  SWLog(INFO, "Learner::FinishLearnValue - learn a new value=%s.\n",
        learned_value_.user_data().c_str());
}

void Learner::BroadcastMessageToFollower(const BallotNumber& ballot) {
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(ballot.GetProposalId());
  msg->set_proposal_node_id(ballot.GetNodeId());
  msg->set_allocated_value(new PaxosValue(learned_value_));
  messager_->BroadcastMessageToFollower(messager_->PackMessage(msg));
}

void Learner::NextInstance() {
  has_learned_ = false;
  learned_value_.Clear();
  ++instance_id_;
}

}  // namespace skywalker
