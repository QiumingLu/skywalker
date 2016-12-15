#include "paxos/learner.h"
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
      hightest_instance_id_(0),
      hightest_instance_id_from_node_id_(0),
      is_learning_(false),
      has_learned_(false) {
}

void Learner::OnNewChosenValue(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    const BallotNumber& b = acceptor_->GetAcceptedBallot();
    BallotNumber ballot(msg.proposal_id(), msg.node_id());
    if (ballot == b) {
      FinishLearnValue(acceptor_->GetAcceptedValue(), b);
    } else if (msg.value().empty() == false) {
      int res = WriteToDB(instance_id_, ballot, msg.value(), 0);
      if (res == 0) {
        FinishLearnValue(msg.value(), b);
      }
    }
  }
  SetHightestInstanceId(msg.instance_id(), msg.node_id());
}

void Learner::AskForLearn() {
  is_learning_ = false;
  PaxosMessage* msg = new PaxosMessage();
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_type(LEARNER_ASK_FOR_LEARN);
  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->BroadcastMessage(content_ptr);
}

void Learner::OnAskForLearn(const PaxosMessage& msg) {
  if (msg.instance_id() < instance_id_) {
    if (msg.instance_id() == instance_id_ - 1) {
      std::string s;
      int res = config_->GetDB()->Get(msg.instance_id(), &s);
      if (res == 0) {
        AcceptorState state;
        state.ParseFromString(s);
        BallotNumber ballot(state.accepted_id(), state.accepted_node_id());
        SendLearnedValue(msg.node_id(), msg.instance_id(),
                         ballot, state.accepted_value());
      }
    } else {
      SendNowInstanceId(msg);
    }
  }
  SetHightestInstanceId(msg.instance_id(), msg.node_id());
}

void Learner::SendNowInstanceId(const PaxosMessage& msg) {
  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_instance_id(msg.instance_id());
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_type(LEARNER_SEND_NOW_INSTANCE_ID);
  reply_msg->set_now_instance_id(instance_id_);
  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, reply_msg, nullptr);
  messager_->SendMessage(msg.node_id(), content_ptr);
}

void Learner::OnSendNowInstanceId(const PaxosMessage& msg) {
 if (msg.instance_id() == instance_id_ &&
     msg.now_instance_id() > instance_id_) {
    if (!is_learning_) {
      ComfirmAskForLearn(msg);
      is_learning_ = true;
    }
  }
  SetHightestInstanceId(msg.now_instance_id(), msg.node_id());
}

void Learner::ComfirmAskForLearn(const PaxosMessage& msg) {
  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(instance_id_);
  reply_msg->set_type(LEARNER_COMFIRM_ASK_FOR_LEARN);
  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, reply_msg, nullptr);
  messager_->SendMessage(msg.node_id(), content_ptr);
}

void Learner::OnComfirmAskForLearn(const PaxosMessage& msg) {
  uint64_t i = msg.instance_id();
  while (i < instance_id_) {
    std::string s;
    int ret = config_->GetDB()->Get(i, &s);
    if (ret == 0) {
      AcceptorState state;
      state.ParseFromString(s);
      BallotNumber ballot(state.accepted_id(), state.accepted_node_id());
      SendLearnedValue(msg.node_id(), i, ballot, state.accepted_value());
      ++i;
    } else {
      SWLog(ERROR, "Learner::OnComfirmAskForLearn - "
            "no found data of instance %" PRIu64"\n",
            i);
      break;
    }
  }
}

void Learner::SendLearnedValue(uint64_t node_id,
                               uint64_t learner_instance_id,
                               const BallotNumber& learned_ballot,
                               const std::string& learned_value) {
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(LEARNER_SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(learner_instance_id);
  msg->set_proposal_id(learned_ballot.GetProposalId());
  msg->set_proposal_node_id(learned_ballot.GetNodeId());
  msg->set_value(learned_value);
  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->SendMessage(node_id, content_ptr);
}

void Learner::OnSendLearnedValue(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    BallotNumber ballot(msg.proposal_id(), msg.proposal_node_id());
    int res = WriteToDB(msg.instance_id(), ballot, msg.value(), 0);
    if (res == 0) {
      FinishLearnValue(msg.value(), ballot);
    }
  }
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

int Learner::WriteToDB(uint64_t instance_id,
                       const BallotNumber& ballot,
                       const std::string& value,
                       uint32_t last_checksum) {
  AcceptorState state;
  state.set_instance_id(instance_id);
  state.set_promised_id(ballot.GetProposalId());
  state.set_promised_node_id(ballot.GetNodeId());
  state.set_accepted_id(ballot.GetProposalId());
  state.set_accepted_node_id(ballot.GetNodeId());
  state.set_accepted_value(value);

  WriteOptions options;
  options.sync = false;

  std::string s;
  state.SerializeToString(&s);
  int res = config_->GetDB()->Put(options, instance_id, s);
  return res;
}

void Learner::FinishLearnValue(const std::string& value,
                               const BallotNumber& ballot) {
  learned_value_ = value;
  has_learned_ = true;
  BroadcastMessageToFollower(ballot);
  SWLog(INFO,
        "Learner::FinishLearnValue - learn a new value=%s.\n",
        learned_value_.c_str());
}

void Learner::BroadcastMessageToFollower(const BallotNumber& ballot) {
  PaxosMessage* msg = new PaxosMessage();
  msg->set_type(LEARNER_SEND_LEARNED_VALUE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id_);
  msg->set_proposal_id(ballot.GetProposalId());
  msg->set_proposal_node_id(ballot.GetNodeId());
  msg->set_value(learned_value_);
  std::shared_ptr<Content> content_ptr =
      messager_->PackMessage(PAXOS_MESSAGE, msg, nullptr);
  messager_->BroadcastMessageToFollower(content_ptr);
}

}  // namespace skywalker
