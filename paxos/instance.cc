#include "paxos/instance.h"
#include "paxos/config.h"
#include "paxos/runloop.h"
#include "util/mutexlock.h"
#include "skywalker/logging.h"

namespace skywalker {

Instance::Instance(Config* config)
    : config_(config),
      loop_(config_->GetLoop()),
      acceptor_(config, this),
      learner_(config, this, &acceptor_),
      proposer_(config, this),
      instance_id_(0),
      is_proposing_(false),
      rand_(301) {
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

  learn_timer_ = loop_->RunEvery(30000 + rand_.Uniform(10000), [this]() {
    learner_.AskForLearn();
  });

  SWLog(INFO,
        "Instance::Init - now instance_id=%" PRIu64".\n", instance_id_);
  return ret;
}

void Instance::HandlePropose(const Slice& value) {
  assert(!is_proposing_);
  propose_value_ = value;
  proposer_.NewPropose(value);
  propose_timer_ = loop_->RunAfter(1000, [this]() {
    proposer_.QuitPropose();
    is_proposing_ = false;
    propose_cb_(-1);
  });
  is_proposing_ = true;
}

void Instance::HandleContent(const std::shared_ptr<Content>& c) {
  if (c->type() == PAXOS_MESSAGE) {
    HandlePaxosMessage(c->paxos_msg());
  } else if (c->type() == CHECKPOINT_MESSAGE) {
    HandleCheckPointMessage(c->checkpoint_msg());
  }
}

void Instance::HandlePaxosMessage(const PaxosMessage& msg) {
  SWLog(DEBUG,
        "Instance::HandlePaxosMessage - "
        "msg.type=%d, msg.node_id=%" PRIu64", msg.instance_id=%" PRIu64", "
        "msg.proposal_id=%" PRIu64", msg.proposal_node_id=%" PRIu64", "
        "msg.value=%s, "
        "msg.pre_accepted_id=%" PRIu64", msg.pre_accepted_node_id=%" PRIu64", "
        "msg.reject_for_promised_id=%" PRIu64", "
        "msg.now_instance_id=%" PRIu64".\n",
        msg.type(), msg.node_id(), msg.instance_id(),
        msg.proposal_id(), msg.proposal_node_id(),
        msg.value().c_str(),
        msg.pre_accepted_id(), msg.pre_accepted_node_id(),
        msg.reject_for_promised_id(),
        msg.now_instance_id());

  switch(msg.type()) {
    case PREPARE_REPLY:
    case ACCEPT_REPLY:
      ProposerHandleMessage(msg);
      break;
    case PREPARE:
    case ACCEPT:
      AcceptorHandleMessage(msg);
      break;
   default:
      LearnerHandleMessage(msg);
      break;
  }
}

void Instance::HandleCheckPointMessage(const CheckPointMessage& msg) {
}

void Instance::ProposerHandleMessage(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    if (msg.type() == PREPARE_REPLY) {
      proposer_.OnPrepareReply(msg);
    } else if (msg.type() == ACCEPT_REPLY) {
      proposer_.OnAccpetReply(msg);
    }
  } else {
    SWLog(DEBUG,
          "Instance::ProposerHandleMessage - "
          "now instance_id=%" PRIu64", but msg.instance_id=%" PRIu64", "
          "they are not same, so skip this msg\n",
          instance_id_, msg.instance_id());
  }
}

void Instance::AcceptorHandleMessage(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    if (msg.type() == PREPARE) {
      acceptor_.OnPrepare(msg);
    } else if (msg.type() == ACCEPT) {
      acceptor_.OnAccpet(msg);
    }
  } else if (msg.instance_id() == instance_id_ + 1) {
    PaxosMessage new_msg;
    new_msg.set_type(NEW_CHOSEN_VALUE);
    new_msg.set_node_id(msg.node_id());
    new_msg.set_instance_id(instance_id_);
    new_msg.set_proposal_id(msg.proposal_id());
    LearnerHandleMessage(new_msg);
  }
}

void Instance::LearnerHandleMessage(const PaxosMessage& msg) {
  switch(msg.type()) {
    case NEW_CHOSEN_VALUE:
      learner_.OnNewChosenValue(msg);
      break;
    case LEARNER_ASK_FOR_LEARN:
      learner_.OnAskForLearn(msg);
      break;
    case LEARNER_SEND_LEARNED_VALUE:
      learner_.OnSendLearnedValue(msg);
      break;
    case LEARNER_SEND_NOW_INSTANCE_ID:
      learner_.OnSendNowInstanceId(msg);
      break;
    case LEARNER_COMFIRM_ASK_FOR_LEARN:
      learner_.OnComfirmAskForLearn(msg);
      break;
    default:
      SWLog(ERROR, "Instance::LearnerHandleMessage - Invalid message type.\n");
      break;
  }

  if (learner_.HasLearned()) {
    if (is_proposing_) {
      int res = 0;
      if (propose_value_ != learner_.GetLearnedValue()) {
        res = 1;
      }
      propose_cb_(res);
      loop_->Remove(propose_timer_);
      is_proposing_ = false;
    }
    NextInstance();
  }
}

void Instance::NextInstance() {
  ++instance_id_;
  acceptor_.NextInstance();
  proposer_.NextInstance();
  learner_.NextInstance();
  SWLog(INFO,
        "Instance::NextInstance - "
        "New instance is starting, Now instance_id=%" PRIu64".\n",
        instance_id_);
}

bool Instance::MachineExecute(uint64_t instance_id, const Slice& value,
                              bool my_proposal, MachineContext* context) {
  return true;
}

}  // namespace skywalker
