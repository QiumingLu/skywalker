#include "voyager/paxos/learner.h"
#include "voyager/paxos/config.h"
#include "voyager/paxos/instance.h"
#include "voyager/util/logging.h"

namespace voyager {
namespace paxos {

Learner::Learner(Config* config, Instance* instance, Acceptor* acceptor)
    : config_(config),
      instance_(instance),
      messager_(config->GetMessager()),
      acceptor_(acceptor),
      instance_id_(0),
      hightest_instance_id_(0),
      hightest_instance_id_from_node_id_(0),
      is_learning_(false),
      has_learned_(false) {
}

void Learner::OnNewChosenValue(const PaxosMessage& msg) {
  if (msg.instance_id() != instance_id_) {
    return;
  }
  const BallotNumber& b = acceptor_->GetAcceptedBallot();
  BallotNumber ballot(msg.proposal_id(), msg.node_id());
  if (ballot == b) {
    instance_id_ = acceptor_->GetInstanceId();
    learned_value_ = acceptor_->GetAcceptedValue();
    has_learned_ = true;
    BroadcastMessageToFollower();
    VOYAGER_LOG(DEBUG) << "Learner::OnNewChosenValue - learn a new chosen value,"
                       << " which node_id=" << msg.node_id()
                       << ", proposal_id=" << msg.proposal_id()
                       << ", and now learn's instance_id_=" << instance_id_
                       << ", learned_value_=" << learned_value_;

  }
}

void Learner::AskForLearn() {
  PaxosMessage* msg = new PaxosMessage();
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_type(LEARNER_ASK_FOR_LEARN);
  Content* content = messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->BroadcastMessage(content);
  delete content;
}

void Learner::OnAskForLearn(const PaxosMessage& msg) {
  SetHightestInstanceId(msg.instance_id(), msg.node_id());

  if (msg.instance_id() < instance_id_) {
    if (msg.instance_id() == instance_id_ - 1) {
      std::string value;
      int res = config_->GetDB()->Get(msg.instance_id(), &value);
      if (res == 0) {
        AcceptorState state;
        state.ParseFromString(value);
        BallotNumber ballot(state.accepted_id(), state.accepted_node_id());
        SendLearnedValue(msg.node_id(), msg.instance_id(),
                         ballot, state.accepted_value());
        return;
      }
    }
    SendNowInstanceId(msg);
  }
}

void Learner::SendNowInstanceId(const PaxosMessage& msg) {
  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_instance_id(msg.instance_id());
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_type(LEARNER_SEND_NOW_INSTANCE_ID);
  reply_msg->set_now_instance_id(instance_id_);
  Content* content =
      messager_->PackMessage(PAXOS_MESSAGE, reply_msg, nullptr);
  messager_->SendMessage(msg.node_id(), content);
  delete content;
}

void Learner::OnSendNowInstanceId(const PaxosMessage& msg) {
  SetHightestInstanceId(msg.now_instance_id(), msg.node_id());

  if (msg.instance_id() == instance_id_ &&
      msg.now_instance_id() > instance_id_) {
    if (!is_learning_) {
      ComfirmAskForLearn(msg);
    }
  }
}

void Learner::ComfirmAskForLearn(const PaxosMessage& msg) {
  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(instance_id_);
  reply_msg->set_type(LEARNER_COMFIRM_ASK_FOR_LEARN);
  Content* content = messager_->PackMessage(PAXOS_MESSAGE, reply_msg, nullptr);
  messager_->SendMessage(msg.node_id(), content);
  delete content;
  is_learning_ = true;
}

void Learner::OnComfirmAskForLearn(const PaxosMessage& msg) {
}

void Learner::SendLearnedValue(uint64_t node_id,
                               uint64_t learner_instance_id,
                               const BallotNumber& learned_ballot,
                               const std::string& learned_value) {
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(LEARNER_SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(learner_instance_id);
  msg->set_proposal_id(learned_ballot.GetNodeId());
  msg->set_proposal_node_id(learned_ballot.GetProposalId());
  msg->set_value(learned_value);
  Content* content = messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->SendMessage(node_id, content);
  delete content;
}

void Learner::OnSendLearnedValue(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    BallotNumber ballot(msg.proposal_id(), msg.proposal_node_id());
    AcceptorState state;
    state.set_instance_id(msg.instance_id());
    state.set_promised_id(msg.proposal_id());
    state.set_promised_node_id(msg.proposal_node_id());
    state.set_accepted_id(msg.proposal_id());
    state.set_accepted_node_id(msg.proposal_node_id());
    state.set_accepted_value(msg.value());

    WriteOptions options;
    options.sync = false;
    std::string value;
    state.SerializeToString(&value);
    int ret = config_->GetDB()->Put(options, instance_id_, value);
    if (ret == 0) {
      has_learned_ = true;
      learned_value_ = value;
    }
  }
}

void Learner::BroadcastMessageToFollower() {
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(LEARNER_SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_node_id(acceptor_->GetAcceptedBallot().GetNodeId());
  msg->set_proposal_id(acceptor_->GetAcceptedBallot().GetProposalId());
  msg->set_value(acceptor_->GetAcceptedValue());
  Content* content = messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->BroadcastMessageToFollower(content);
  delete content;
}

void Learner::NextInstance() {
  ++instance_id_;
  has_learned_ = false;
  learned_value_.clear();
}

void Learner::SetHightestInstanceId(uint64_t instance_id,
                                    uint64_t node_id) {
  if (instance_id > hightest_instance_id_) {
    hightest_instance_id_ = instance_id;
    hightest_instance_id_from_node_id_ = node_id;
  }
}

}  // namespace paxos
}  // namespace voyager
