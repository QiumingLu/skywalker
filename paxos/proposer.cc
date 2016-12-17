#include "paxos/proposer.h"
#include "paxos/config.h"
#include "paxos/instance.h"
#include "skywalker/logging.h"

namespace skywalker {

Proposer::Proposer(Config* config, Instance* instance)
    : config_(config),
      instance_(instance),
      messager_(config->GetMessager()),
      hightest_proprosal_id_(0),
      instance_id_(0),
      proposal_id_(0),
      counter_(config),
      preparing_(false),
      accepting_(false),
      skip_prepare_(false),
      was_rejected_by_someone_(false),
      loop_(config_->GetLoop()),
      rand_(301) {
}

void Proposer::NewPropose(const PaxosValue& value) {
  value_ = value;

  if (skip_prepare_ && !was_rejected_by_someone_) {
    Accept();
  } else {
    Prepare(was_rejected_by_someone_);
  }
}

void Proposer::Prepare(bool need_new_ballot) {
  preparing_ = true;
  accepting_ = false;
  skip_prepare_ = false;
  was_rejected_by_someone_ = false;

  if (need_new_ballot) {
    if (proposal_id_ < hightest_proprosal_id_) {
      proposal_id_ = hightest_proprosal_id_;
    }
    proposal_id_ += 1;
  }

  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(PREPARE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);

  counter_.StartNewRound();

  SWLog(DEBUG,
        "Proposer::Prepare - start a new prepare, now "
        "node_id=%" PRIu64", instance_id=%" PRIu64", "
        "proposal_id=%" PRIu64", value=%s.\n",
        config_->GetNodeId(), instance_id_,
        proposal_id_, value_.user_data().c_str());

  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->BroadcastMessage(content_ptr);
  instance_->HandlePaxosMessage(content_ptr->paxos_msg());

  AddRetryTimer();
}


void Proposer::OnPrepareReply(const PaxosMessage& msg) {
  SWLog(DEBUG,
        "Proposer::OnPrepareReply - receive the prepare reply, "
        "which node_id=%" PRIu64", proposal_id=%" PRIu64", "
        "reject_for_promised_id=%" PRIu64", "
        "pre_accepted_id=%" PRIu64", pre_accepted_node_id=%" PRIu64", "
        "value=%s.\n",
        msg.node_id(), msg.proposal_id(), msg.reject_for_promised_id(),
        msg.pre_accepted_id(), msg.pre_accepted_node_id(),
        msg.value().user_data().c_str());

  if (preparing_) {
    if (msg.proposal_id() == proposal_id_) {
      counter_.AddReceivedNode(msg.node_id());

      if (msg.reject_for_promised_id() == 0) {
        counter_.AddPromisorOrAcceptor(msg.node_id());
        BallotNumber b(msg.pre_accepted_id(), msg.pre_accepted_node_id());
        if (b > hightest_ballot_) {
          hightest_ballot_ = b;
          value_ = msg.value();
        }
      } else {
        counter_.AddRejector(msg.node_id());
        was_rejected_by_someone_ = true;
        if (hightest_proprosal_id_ < msg.reject_for_promised_id()) {
          hightest_proprosal_id_ = msg.reject_for_promised_id();
        }
      }

      if (counter_.IsPassedOnThisRound()) {
        SWLog(DEBUG, "Proposer::OnPrepareReply - Prepare pass.\n");
        skip_prepare_ = true;
        Accept();
      } else if (counter_.IsRejectedOnThisRound() ||
                 counter_.IsReceiveAllOnThisRound()) {
        SWLog(DEBUG, "Proposer::OnPrepareReply - "
              "Prepare not pass, reprepare about 30ms later.\n");
        AddRetryTimer(rand_.Uniform(30) + 10);
      }
    }
  }
}

void Proposer::Accept() {
  SWLog(DEBUG,
        "Proposer::Accept - start to accept, "
        "now node_id=%" PRIu64", instance_id=%" PRIu64", "
        "proposal_id=%" PRIu64", value=%s.\n",
        config_->GetNodeId(), instance_id_, proposal_id_,
        value_.user_data().c_str());

  preparing_ = false;
  accepting_ = true;

  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(ACCEPT);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(proposal_id_);
  msg->set_allocated_value(new PaxosValue(value_));

  counter_.StartNewRound();

  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->BroadcastMessage(content_ptr);
  instance_->HandlePaxosMessage(content_ptr->paxos_msg());

  AddRetryTimer();
}

void Proposer::OnAccpetReply(const PaxosMessage& msg) {
  SWLog(DEBUG,
        "Proposer::OnAccpetReply - receive the accept reply, "
        "which node_id=%" PRIu64", "
        "proposal_id=%" PRIu64", "
        "reject_for_promised_id=%" PRIu64".\n",
        msg.node_id(), msg.proposal_id(), msg.reject_for_promised_id());

  if (accepting_) {
    if (msg.proposal_id() == proposal_id_) {
      counter_.AddReceivedNode(msg.node_id());
      if (msg.reject_for_promised_id() == 0) {
        counter_.AddPromisorOrAcceptor(msg.node_id());
      } else {
        counter_.AddRejector(msg.node_id());
        was_rejected_by_someone_ = true;
        if (hightest_proprosal_id_ < msg.reject_for_promised_id()) {
          hightest_proprosal_id_ = msg.reject_for_promised_id();
        }
      }

      if (counter_.IsPassedOnThisRound()) {
        SWLog(DEBUG, "Proposer::OnAccpetReply - Accept pass.\n");
        QuitPropose();
        NewChosenValue();
      } else if (counter_.IsRejectedOnThisRound() ||
                 counter_.IsReceiveAllOnThisRound()) {
        SWLog(DEBUG, "Proposer::OnAccpetReply - "
              "Accept not pass, reprepare 30ms later.\n");
        AddRetryTimer(rand_.Uniform(30) + 10);
      }
    }
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
  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->BroadcastMessage(content_ptr);
  instance_->HandlePaxosMessage(content_ptr->paxos_msg());
}

void Proposer::AddRetryTimer(uint64_t timeout) {
  uint64_t id = instance_id_;
  loop_->Remove(retry_timer_);
  retry_timer_ = loop_->RunAfter(timeout, [id, this]() {
    if (id == instance_id_) {
      Prepare(was_rejected_by_someone_);
    }
  });
}

void Proposer::QuitPropose() {
  preparing_ = false;
  accepting_ = false;
  loop_->Remove(retry_timer_);
}

void Proposer::NextInstance() {
  hightest_ballot_.Reset();
  hightest_proprosal_id_ = 0;
  preparing_ = false;
  accepting_ = false;
  counter_.StartNewRound();
  ++instance_id_;
}

}  // namespace skywalker
