#include "paxos/instance.h"
#include "util/mutexlock.h"
#include "skywalker/logging.h"

namespace skywalker {

Instance::Instance(Config* config)
    : config_(config),
      acceptor_(config, this),
      learner_(config, this, &acceptor_),
      proposer_(config, this),
      loop_(this),
      mutex_(),
      transfer_(config, &loop_) {
}

Instance::~Instance() {
  loop_.Exit();
}

bool Instance::Init() {
  bool ret = acceptor_.Init();
  if (!ret) {
    SWLog(ERROR, "Instance::Init - Acceptor init fail.\n");
    return ret;
  }
  uint64_t now_instance_id = acceptor_.GetInstanceId();
  learner_.SetInstanceId(now_instance_id);
  proposer_.SetInstanceId(now_instance_id);
  proposer_.SetStartProposalId(
      acceptor_.GetPromisedBallot().GetProposalId() + 1);

  loop_.Loop();

  return ret;
}

bool Instance::OnReceiveValue(const Slice& value,
                              MachineContext* context,
                              uint64_t* new_instance_id) {
  MutexLock lock(&mutex_);
  return transfer_.NewValue(value, context, new_instance_id);
}

void Instance::OnReceiveContent(Content* content) {
  loop_.NewContent(content);
}

void Instance::HandleNewValue(const std::string& value) {
  transfer_.SetNowInstanceId(proposer_.GetInstanceId());
  proposer_.NewValue(value);
}

void Instance::HandleContent(const Content& content) {
  if (content.type() == PAXOS_MESSAGE) {
    HandlePaxosMessage(content.paxos_msg());
  } else if (content.type() == CHECKPOINT_MESSAGE) {
    HandleCheckPointMessage(content.checkpoint_msg());
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
    case PROPOSER_SEND_NEW_VALUE:
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
  if (msg.instance_id() == proposer_.GetInstanceId()) {
    if (msg.type() == PREPARE_REPLY) {
      proposer_.OnPrepareReply(msg);
    } else if (msg.type() == ACCEPT_REPLY) {
      proposer_.OnAccpetReply(msg);
    }
  } else {
    SWLog(DEBUG,
          "Instance::ProposerHandleMessage - "
          "now proposer.instance_id=%" PRIu64", but msg.instance_id=%" PRIu64", "
          "they are not same, so skip this msg\n",
          proposer_.GetInstanceId(), msg.instance_id());
  }
}

void Instance::AcceptorHandleMessage(const PaxosMessage& msg) {
  if (msg.instance_id() == acceptor_.GetInstanceId() + 1) {
    PaxosMessage new_msg(msg);
    new_msg.set_instance_id(acceptor_.GetInstanceId());
    new_msg.set_type(NEW_CHOSEN_VALUE);
    LearnerHandleMessage(new_msg);
  }

  if (msg.instance_id() == acceptor_.GetInstanceId()) {
    if (msg.type() == PREPARE) {
      acceptor_.OnPrepare(msg);
    } else if (msg.type() == ACCEPT) {
      acceptor_.OnAccpet(msg);
    }
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
    MachineContext* context = nullptr;
    bool my_proposal = transfer_.IsMyProposal(learner_.GetInstanceId(),
                                              learner_.GetLearnedValue(),
                                              &context);
    bool success = MachineExecute(learner_.GetInstanceId(),
                                  learner_.GetLearnedValue(),
                                  my_proposal,
                                  context);
    transfer_.SetResult(success, learner_.GetInstanceId(),
                        learner_.GetLearnedValue());

    if (success) {
      NextInstance();
    } else {
      proposer_.SetNoSkipPrepare();
    }
  }
}

void Instance::NextInstance() {
  acceptor_.NextInstance();
  proposer_.NextInstance();
  learner_.NextInstance();
  SWLog(INFO,
        "Instance::NextInstance - New instance is starting, "
        "Now proposer.instance_id=%" PRIu64", "
        "acceptor.instance_id=%" PRIu64", "
        "learner.instance_id=%" PRIu64".\n",
        proposer_.GetInstanceId(),
        acceptor_.GetInstanceId(),
        learner_.GetInstanceId());
}

bool Instance::MachineExecute(uint64_t instance_id, const Slice& value,
                              bool my_proposal, MachineContext* context) {
  return true;
}

}  // namespace skywalker
