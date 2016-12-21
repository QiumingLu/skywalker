#include "paxos/proposer.h"
#include "paxos/config.h"
#include "paxos/instance.h"
#include "skywalker/logging.h"

namespace skywalker {

Proposer::Proposer(Config* config, Instance* instance)
    : config_(config),
      instance_(instance),
      messager_(config->GetMessager()),
      counter_(config),
      instance_id_(0),
      proposal_id_(0),
      max_proprosal_id_(0),
      max_ballot_(),
      value_(),
      preparing_(false),
      accepting_(false),
      skip_prepare_(false),
      was_rejected_by_someone_(false),
      rand_(0xdeadbeef) {
}

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

  SWLog(DEBUG,
        "Proposer::Prepare - start a new prepare, now "
        "node_id=%" PRIu64", instance_id=%" PRIu64", "
        "proposal_id=%" PRIu64", value=%s.\n",
        config_->GetNodeId(), instance_id_,
        proposal_id_, value_.user_data().c_str());

  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(PREPARE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);

  counter_.StartNewRound();
  AddRetryTimer();

  std::shared_ptr<Content> c(messager_->PackMessage(msg));
  messager_->BroadcastMessage(c);
  instance_->OnPaxosMessage(c->paxos_msg());
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
      SWLog(DEBUG, "Proposer::OnPrepareReply - Prepare pass.\n");
      preparing_ = false;
      skip_prepare_ = true;
      RemoveRetryTimer();
      Accept();
    } else if (counter_.IsRejectedOnThisRound() ||
               counter_.IsReceiveAllOnThisRound()) {
      SWLog(DEBUG, "Proposer::OnPrepareReply - "
            "Prepare not pass, reprepare about 20ms later.\n");
      preparing_ = false;
      RemoveRetryTimer();
      AddRetryTimer(rand_.Uniform(10) + 10);
    }
  }
  SetMaxProposalId(msg);
}

void Proposer::Accept() {
  preparing_ = false;
  accepting_ = true;
  SWLog(DEBUG,
        "Proposer::Accept - start to accept, "
        "now node_id=%" PRIu64", instance_id=%" PRIu64", "
        "proposal_id=%" PRIu64", value=%s.\n",
        config_->GetNodeId(), instance_id_, proposal_id_,
        value_.user_data().c_str());

  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(ACCEPT);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);
  msg->set_allocated_value(new PaxosValue(value_));

  counter_.StartNewRound();
  AddRetryTimer();

  std::shared_ptr<Content> c(messager_->PackMessage(msg));
  messager_->BroadcastMessage(c);
  instance_->OnPaxosMessage(c->paxos_msg());
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
      SWLog(DEBUG, "Proposer::OnAccpetReply - Accept pass.\n");
      accepting_ = false;
      RemoveRetryTimer();
      NewChosenValue();
    } else if (counter_.IsRejectedOnThisRound() ||
               counter_.IsReceiveAllOnThisRound()) {
      SWLog(DEBUG, "Proposer::OnAccpetReply - "
            "Accept not pass, reprepare about 20ms later.\n");
      accepting_ = false;
      RemoveRetryTimer();
      AddRetryTimer(rand_.Uniform(10) + 10);
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
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(NEW_CHOSEN_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);
  if (value_.ByteSizeLong() <= 128) {
    msg->set_allocated_value(new PaxosValue(value_));
  }
  std::shared_ptr<Content> c(messager_->PackMessage(msg));
  messager_->BroadcastMessage(c);
  instance_->OnPaxosMessage(c->paxos_msg());
}

void Proposer::AddRetryTimer(uint64_t timeout) {
  uint64_t id = instance_id_;
  retry_timer_ = config_->GetLoop()->RunAfter(timeout, [id, this]() {
    if (id == instance_id_) {
      Prepare(was_rejected_by_someone_);
    }
  });
}

void Proposer::RemoveRetryTimer() {
  config_->GetLoop()->Remove(retry_timer_);
}

void Proposer::QuitPropose() {
  preparing_ = false;
  accepting_ = false;
  RemoveRetryTimer();
}

void Proposer::NextInstance() {
  ++instance_id_;
}

}  // namespace skywalker
