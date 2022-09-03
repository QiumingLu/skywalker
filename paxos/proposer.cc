// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/proposer.h"

#include <memory>

#include "paxos/config.h"
#include "paxos/instance.h"
#include "skywalker/logging.h"
#include "util/timeops.h"

namespace skywalker {

Proposer::Proposer(Config* config, Instance* instance)
    : config_(config),
      instance_(instance),
      messager_(config->GetMessager()),
      counter_(config),
      instance_id_(1),
      proposal_id_(0),
      max_proprosal_id_(0),
      max_ballot_(),
      value_(),
      preparing_(false),
      accepting_(false),
      skip_prepare_(false),
      was_rejected_by_someone_(false),
      rand_(static_cast<uint32_t>(NowMillis())) {}

void Proposer::NewPropose(const PaxosValue& value) {
  value_ = value;
  if (skip_prepare_ && !was_rejected_by_someone_) {
    Accept();
  } else {
    Prepare(was_rejected_by_someone_);
  }
}

void Proposer::Prepare(bool need_new_proposal_id) {
  preparing_ = true;
  accepting_ = false;
  skip_prepare_ = false;
  was_rejected_by_someone_ = false;
  max_ballot_.Reset();

  if (need_new_proposal_id) {
    if (proposal_id_ < max_proprosal_id_) {
      proposal_id_ = max_proprosal_id_;
    }
    proposal_id_ += 1;
  }

  LOG_DEBUG(
      "Group %u - start to prepare, now instance_id=%llu, proposal_id=%llu",
      config_->GetGroupId(), (unsigned long long)instance_id_,
      (unsigned long long)proposal_id_);

  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* msg = content.mutable_paxos_msg();
  msg->set_type(PREPARE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);

  counter_.StartNewRound();
  AddRetryTimer();

  messager_->BroadcastMessage(content);
  instance_->OnPaxosMessage(*msg);
}

void Proposer::OnPrepareReply(const PaxosMessage& msg) {
  if (preparing_ && (msg.instance_id() == instance_id_)) {
    counter_.AddReceivedNode(msg.node_id());

    if (msg.rejected_id() == 0) {
      counter_.AddPromisorOrAcceptor(msg.node_id());
      BallotNumber b(msg.pre_accepted_id(), msg.pre_accepted_node_id());
      if (b > max_ballot_) {
        max_ballot_ = b;
        value_ = msg.value();
      }
    } else {
      counter_.AddRejector(msg.node_id());
    }

    if (counter_.IsPassedOnThisRound()) {
      LOG_DEBUG("Group %u - prepare pass.", config_->GetGroupId());
      preparing_ = false;
      skip_prepare_ = true;
      RemoveRetryTimer();
      Accept();
    } else if (counter_.IsRejectedOnThisRound() ||
               counter_.IsReceiveAllOnThisRound()) {
      LOG_DEBUG("Group %u - prepare not pass, reprepare about 30ms later.",
                config_->GetGroupId());
      preparing_ = false;
      RemoveRetryTimer();
      AddRetryTimer((rand_.Uniform(15) + 15) * 1000);
    }
  }
  SetMaxProposalId(msg);
}

void Proposer::Accept() {
  preparing_ = false;
  accepting_ = true;
  LOG_DEBUG(
      "Group %u - start to accept, the instance_id=%llu, proposal_id=%llu.",
      config_->GetGroupId(), (unsigned long long)instance_id_,
      (unsigned long long)proposal_id_);

  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* msg = content.mutable_paxos_msg();
  msg->set_type(ACCEPT);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);
  *(msg->mutable_value()) = value_;

  counter_.StartNewRound();
  AddRetryTimer();

  messager_->BroadcastMessage(content);
  instance_->OnPaxosMessage(*msg);
}

void Proposer::OnAccpetReply(const PaxosMessage& msg) {
  if (accepting_ && (msg.instance_id() == instance_id_)) {
    counter_.AddReceivedNode(msg.node_id());
    if (msg.rejected_id() == 0) {
      counter_.AddPromisorOrAcceptor(msg.node_id());
    } else {
      counter_.AddRejector(msg.node_id());
    }

    if (counter_.IsPassedOnThisRound()) {
      LOG_DEBUG("Group %u - accept pass.", config_->GetGroupId());
      accepting_ = false;
      RemoveRetryTimer();
      NewChosenValue();
    } else if (counter_.IsRejectedOnThisRound() ||
               counter_.IsReceiveAllOnThisRound()) {
      LOG_DEBUG("Group %u - accept not pass, reprepare about 30ms later.",
                config_->GetGroupId());
      accepting_ = false;
      RemoveRetryTimer();
      AddRetryTimer((rand_.Uniform(15) + 15) * 1000);
    }
  }

  SetMaxProposalId(msg);
}

void Proposer::SetMaxProposalId(const PaxosMessage& msg) {
  if (msg.rejected_id() >= proposal_id_) {
    was_rejected_by_someone_ = true;
  }
  if (max_proprosal_id_ < msg.rejected_id()) {
    max_proprosal_id_ = msg.rejected_id();
  }
}

void Proposer::NewChosenValue() {
  Content content;
  content.set_type(PAXOS_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  PaxosMessage* msg = content.mutable_paxos_msg();
  msg->set_type(NEW_CHOSEN_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);
  messager_->BroadcastMessage(content);
  instance_->OnPaxosMessage(*msg);
}

void Proposer::AddRetryTimer(uint64_t timeout) {
  uint64_t id = instance_id_;
  retry_timer_ = io_loop_->RunAfter(timeout, [id, this]() {
    if (id == instance_id_) {
      Prepare(was_rejected_by_someone_);
    }
  });
}

void Proposer::RemoveRetryTimer() { io_loop_->Remove(retry_timer_); }

void Proposer::QuitPropose() {
  preparing_ = false;
  accepting_ = false;
  RemoveRetryTimer();
}

void Proposer::NextInstance() { ++instance_id_; }

}  // namespace skywalker
