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
      is_proposing_(false) {
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

  SWLog(INFO,
        "Instance::Init - now instance_id=%" PRIu64".\n", instance_id_);
  return ret;
}

void Instance::AddMachine(StateMachine* machine) {
  MutexLock lock(&mutex_);
  machines_.insert(std::make_pair(machine->GetMachineId(), machine));
}

void Instance::RemoveMachine(StateMachine* machine) {
  MutexLock lock(&mutex_);
  machines_.erase(machine->GetMachineId());
}

void Instance::OnPropose(const Slice& value, int machine_id) {
  assert(!is_proposing_);

  propose_value_.set_machine_id(machine_id);
  propose_value_.set_user_data(value.data(), value.size());
  proposer_.NewPropose(propose_value_);

  propose_timer_ = loop_->RunAfter(1000, [this]() {
    proposer_.QuitPropose();
    is_proposing_ = false;
    propose_cb_(-1, instance_id_);
  });
  is_proposing_ = true;
}

void Instance::OnReceiveContent(const std::shared_ptr<Content>& c) {
  switch(c->type()) {
    case PAXOS_MESSAGE:
      OnPaxosMessage(c->paxos_msg());
      break;
    case CHECKPOINT_MESSAGE:
      OnCheckPointMessage(c->checkpoint_msg());
      break;
    default:
      SWLog(ERROR, "Instance::OnReceiveContent - Invalid content type.\n");
      break;
  }
}

void Instance::OnPaxosMessage(const PaxosMessage& msg) {
  SWLog(DEBUG,
        "Instance::OnPaxosMessage - "
        "msg.type=%d, msg.node_id=%" PRIu64", msg.instance_id=%" PRIu64", "
        "msg.proposal_id=%" PRIu64", msg.proposal_node_id=%" PRIu64", "
        "msg.value=%s, "
        "msg.pre_accepted_id=%" PRIu64", msg.pre_accepted_node_id=%" PRIu64", "
        "msg.reject_for_promised_id=%" PRIu64", "
        "msg.now_instance_id=%" PRIu64".\n",
        msg.type(), msg.node_id(), msg.instance_id(),
        msg.proposal_id(), msg.proposal_node_id(),
        msg.value().user_data().c_str(),
        msg.pre_accepted_id(), msg.pre_accepted_node_id(),
        msg.reject_for_promised_id(),
        msg.now_instance_id());

  switch(msg.type()) {
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
    default:
      SWLog(ERROR, "Instance::OnPaxosMessage - Invalid message type.\n");
      break;
  }

  CheckLearn();
}

void Instance::OnCheckPointMessage(const CheckPointMessage& msg) {
  // TODO
}

void Instance::CheckLearn() {
  if (learner_.HasLearned()) {
    const PaxosValue& learned_value(learner_.GetLearnedValue());
    bool success = MachineExecute(learned_value);

    if (is_proposing_) {
      if (success) {
       if (propose_value_.machine_id() == learned_value.machine_id() &&
           propose_value_.user_data() == learned_value.user_data()) {
          propose_cb_(0, instance_id_);
        } else {
          propose_cb_(1, instance_id_);
        }
      } else {
        propose_cb_(2, instance_id_);
      }
      loop_->Remove(propose_timer_);
      is_proposing_ = false;
    }

    if (success) {
      NextInstance();
    } else {
      proposer_.SetNoSkipPrepare();
    }
  }
}

bool Instance::MachineExecute(const PaxosValue& value) {
  int id = value.machine_id();
  if (id != -1) {
    MutexLock lock(&mutex_);
    auto i = machines_.find(value.machine_id());
    if (i != machines_.end()) {
      assert(i->second != nullptr);
      return i->second->Execute(
          config_->GetGroupId(), instance_id_, value.user_data());
    }
  }
  return true;
}

void Instance::NextInstance() {
  ++instance_id_;
  acceptor_.NextInstance();
  proposer_.NextInstance();
  learner_.NextInstance();
  SWLog(INFO, "Instance::NextInstance - new instance is starting, "
        "now instance_id=%" PRIu64".\n", instance_id_);
}

}  // namespace skywalker
