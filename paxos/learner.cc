// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/learner.h"

#include <memory>
#include <string>

#include "paxos/acceptor.h"
#include "paxos/config.h"
#include "paxos/instance.h"
#include "skywalker/logging.h"
#include "util/timeops.h"

namespace skywalker {

std::atomic<bool> Learner::is_sending_checkpoint_(false);

Learner::Learner(Config* config, Instance* instance, Acceptor* acceptor)
    : config_(config),
      messager_(config_->GetMessager()),
      instance_(instance),
      acceptor_(acceptor),
      instance_id_(1),
      rand_(static_cast<uint32_t>(NowMillis())),
      is_learning_(false),
      has_learned_(false),
      is_receiving_checkponit_(false) {}

void Learner::OnNewChosenValue(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    const BallotNumber& b = acceptor_->GetAcceptedBallot();
    BallotNumber ballot(msg.proposal_id(), msg.node_id());
    if (ballot == b) {
      FinishLearnValue(acceptor_->GetAcceptedValue());
    }
  }
}

void Learner::AskForLearn(bool add_timer) {
  is_learning_ = false;
  is_receiving_checkponit_ = false;
  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* msg = content.mutable_paxos_msg();
  msg->set_type(ASK_FOR_LEARN);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  messager_->BroadcastMessageForLearn(content);

  if (add_timer) {
    AddLearnTimer(30 * 1000 + rand_.Uniform(10 * 1000));
  }
}

void Learner::AddLearnTimer(uint64_t timeout) {
  learn_timer_ = io_loop_->RunAfter(timeout, [this]() { AskForLearn(true); });
}

void Learner::RemoveLearnTimer() { io_loop_->Remove(learn_timer_); }

void Learner::OnAskForLearn(const PaxosMessage& msg) {
  if (msg.instance_id() < instance_id_) {
    if (msg.instance_id() == instance_id_ - 1) {
      std::string s;
      int res = config_->GetDB()->Get(msg.instance_id(), &s);
      if (res == 0) {
        PaxosInstance temp;
        temp.ParseFromString(s);
        SendLearnedValue(msg.node_id(), temp);
      }
    } else if (!is_sending_checkpoint_) {
      SendNowInstanceId(msg);
    }
  }
}

void Learner::SendNowInstanceId(const PaxosMessage& msg) {
  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* reply_msg = content.mutable_paxos_msg();
  reply_msg->set_type(SEND_NOW_INSTANCE_ID);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(msg.instance_id());
  reply_msg->set_now_instance_id(instance_id_);
  reply_msg->set_min_chosen_instance_id(
      config_->GetLogManager()->GetMinChosenInstanceId());

  // in order to make it run in learn loop.
  uint64_t node_id = msg.node_id();
  config_->GetLearnLoop()->QueueInLoop([this, node_id, content = std::move(content)]() {
    messager_->SendMessage(node_id, content);
  });
}

void Learner::OnSendNowInstanceId(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_ &&
      msg.now_instance_id() > instance_id_) {
    if (msg.min_chosen_instance_id() > instance_id_) {
      if (!is_receiving_checkponit_) {
        AskForCheckpoint(msg);
        is_receiving_checkponit_ = true;
      }
    } else {
      if (!is_receiving_checkponit_ && !is_learning_) {
        ComfirmAskForLearn(msg);
        is_learning_ = true;
      }
    }
  }
}

void Learner::ComfirmAskForLearn(const PaxosMessage& msg) {
  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* reply_msg = content.mutable_paxos_msg();
  reply_msg->set_type(COMFIRM_ASK_FOR_LEARN);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(instance_id_);
  messager_->SendMessage(msg.node_id(), content);
}

void Learner::OnComfirmAskForLearn(const PaxosMessage& msg) {
  uint64_t node_id = msg.node_id();
  uint64_t from = msg.instance_id();
  uint64_t to = instance_id_;
  config_->GetLearnLoop()->QueueInLoop(
      [node_id, from, to, this] { ASyncSend(node_id, from, to); });
}

void Learner::ASyncSend(uint64_t node_id, uint64_t from, uint64_t to) {
  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* msg = content.mutable_paxos_msg();
  msg->set_type(SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());

  while (from < to) {
    std::string s;
    int ret = config_->GetDB()->Get(from, &s);
    if (ret == 0) {
      PaxosInstance temp;
      temp.ParseFromString(s);
      msg->set_instance_id(temp.instance_id());
      msg->set_proposal_id(temp.accepted_id());
      msg->set_proposal_node_id(temp.accepted_node_id());
      msg->set_allocated_value(temp.release_accepted_value());
      messager_->SendMessage(node_id, content);
      ++from;
    } else {
      LOG_ERROR("Group %u - no found data of instance %llu",
                config_->GetGroupId(), (unsigned long long)from);
      break;
    }
  }
}

void Learner::SendLearnedValue(uint64_t node_id, const PaxosInstance& p) {
  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* msg = content.mutable_paxos_msg();
  msg->set_type(SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(p.instance_id());
  msg->set_proposal_id(p.accepted_id());
  msg->set_proposal_node_id(p.accepted_node_id());
  *(msg->mutable_value()) = p.accepted_value();
  messager_->SendMessage(node_id, content);
}

void Learner::OnSendLearnedValue(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    if (WriteToDB(msg)) {
      FinishLearnValue(msg.value());
    }
  }
}

void Learner::AskForCheckpoint(const PaxosMessage& msg) {
  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* reply_msg = content.mutable_paxos_msg();
  reply_msg->set_type(ASK_FOR_CHECKPOINT);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(instance_id_);
  messager_->SendMessage(msg.node_id(), content);
}

void Learner::OnAskForCheckpoint(const PaxosMessage& msg) {
  uint64_t node_id = msg.node_id();
  config_->GetLearnLoop()->QueueInLoop([this, node_id] {
    is_sending_checkpoint_ = true;
    SendCheckpoint(node_id);
    is_sending_checkpoint_ = false;
  });
}

void Learner::SendCheckpoint(uint64_t node_id) {
  config_->GetCheckpointManager()->SendCheckpoint(node_id);
}

void Learner::OnSendCheckpoint(const CheckpointMessage& msg) {
  bool success = config_->GetCheckpointManager()->ReceiveCheckpoint(msg);
  uint64_t timeout = success ? 180 * 1000 : 10;
  RemoveLearnTimer();
  AddLearnTimer(timeout);
}

bool Learner::WriteToDB(const PaxosMessage& msg) {
  PaxosInstance temp;
  temp.set_instance_id(msg.instance_id());
  temp.set_promised_id(msg.proposal_id());
  temp.set_promised_node_id(msg.node_id());
  temp.set_accepted_id(msg.proposal_id());
  temp.set_accepted_node_id(msg.node_id());
  *(temp.mutable_accepted_value()) = msg.value();

  WriteOptions options;
  options.sync = false;

  int res = config_->GetDB()->Put(options, msg.instance_id(),
                                  temp.SerializeAsString());
  return res == 0;
}

void Learner::FinishLearnValue(const PaxosValue& value) {
  learned_value_ = value;
  has_learned_ = true;
  LOG_INFO("Group %u - learn a new value.", config_->GetGroupId());
}

void Learner::NextInstance() {
  config_->GetLogManager()->SetMaxChosenInstanceId(instance_id_);
  has_learned_ = false;
  learned_value_.Clear();
  ++instance_id_;
}

}  // namespace skywalker
