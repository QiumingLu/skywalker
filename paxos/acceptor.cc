#include "voyager/paxos/acceptor.h"
#include "voyager/paxos/config.h"
#include "voyager/paxos/instance.h"
#include "voyager/util/logging.h"

namespace voyager {
namespace paxos {

Acceptor::Acceptor(Config* config, Instance* instance)
    : log_sync_count_(0),
      config_(config),
      instance_(instance),
      messager_(config->GetMessager()),
      instance_id_(0) {
}

bool Acceptor::Init() {
  uint64_t instance_id = 0;
  int res = ReadFromDB(&instance_id);
  if (res != 0) {
    return false;
  }

  instance_id_ = instance_id;
  VOYAGER_LOG(DEBUG) << "Acceptor::Init - now instance_id=" << instance_id_;
  return true;
}

void Acceptor::OnPrepare(const PaxosMessage& msg) {
  VOYAGER_LOG(DEBUG) << "Acceptor::OnPrepare - receive a new message, "
                     << "which node_id=" << msg.node_id()
                     << ", instance_id=" << msg.instance_id()
                     << ", proposal_id=" << msg.proposal_id();
  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_type(PREPARE_REPLY);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(instance_id_);
  reply_msg->set_proposal_id(msg.proposal_id());

  BallotNumber b(msg.proposal_id(), msg.node_id());

  if (b >= promised_ballot_) {
    reply_msg->set_pre_accepted_id(accepted_ballot_.GetProposalId());
    reply_msg->set_pre_accepted_node_id(accepted_ballot_.GetNodeId());
    if (accepted_ballot_.GetProposalId() > 0) {
      reply_msg->set_value(accepted_value_);
    }
    promised_ballot_ =  b;
    int ret = WriteToDB(instance_id_, 0);
    if (ret != 0) {
      VOYAGER_LOG(ERROR) << "Acceptor::OnPrepare - write instance_id_ = "
                         << instance_id_ << " to db failed.";
    }
  } else {
    reply_msg->set_reject_for_promised_id(promised_ballot_.GetProposalId());
  }

  if (msg.node_id() == config_->GetNodeId()) {
    instance_->HandlePaxosMessage(*reply_msg);
    delete reply_msg;
  } else {
    Content* content = messager_->PackMessage(PAXOS_MESSAGE, reply_msg, nullptr);
    config_->GetMessager()->SendMessage(msg.node_id(), content);
    delete content;
  }
}

void Acceptor::OnAccpet(const PaxosMessage& msg) {
  VOYAGER_LOG(DEBUG) << "Acceptor::OnAccpet - receive a new message, "
                     << "which node_id=" << msg.node_id()
                     << ", instance_id=" << msg.instance_id()
                     << ", proposal_id=" << msg.proposal_id();

  PaxosMessage* reply_msg = new PaxosMessage();
  reply_msg->set_type(ACCEPT_REPLY);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_instance_id(instance_id_);
  reply_msg->set_proposal_id(msg.proposal_id());

  BallotNumber b(msg.proposal_id(), msg.node_id());
  if (b >= promised_ballot_) {
    promised_ballot_ = b;
    accepted_ballot_ = b;
    accepted_value_ = msg.value();
    int ret = WriteToDB(instance_id_, 0);
    if (ret != 0) {
      VOYAGER_LOG(ERROR) << "Acceptor::OnAccpet - write instance_id_ = "
                         << instance_id_ << " to db failed.";
    }

  } else {
    reply_msg->set_reject_for_promised_id(promised_ballot_.GetProposalId());
  }

  if (msg.node_id() == config_->GetNodeId()) {
    instance_->HandlePaxosMessage(*reply_msg);
    delete reply_msg;
  } else {
    Content* content = messager_->PackMessage(PAXOS_MESSAGE, reply_msg, nullptr);
    messager_->SendMessage(msg.node_id(), content);
    delete content;
  }
}

void Acceptor::NextInstance() {
  ++instance_id_;
  promised_ballot_.Reset();
  accepted_ballot_.Reset();
  accepted_value_.clear();
}

int Acceptor::ReadFromDB(uint64_t* instance_id) {
  int res = config_->GetDB()->GetMaxInstanceId(instance_id);
  if (res == 1) {
    *instance_id = 1;
    return 0;
  }

  std::string value;
  res = config_->GetDB()->Get(*instance_id, &value);
  if (res !=  0) {
    *instance_id = 1;
    return res;
  }

  AcceptorState state;
  state.ParseFromString(value);
  promised_ballot_.SetProposalId(state.promised_id());
  promised_ballot_.SetNodeId(state.promised_node_id());
  accepted_ballot_.SetProposalId(state.accepted_id());
  accepted_ballot_.SetNodeId(state.accepted_node_id());
  accepted_value_ = std::move(state.accepted_value());
  return 0;
}

int Acceptor::WriteToDB(uint64_t instance_id, uint32_t last_checksum) {
  AcceptorState state;
  state.set_instance_id(instance_id);
  state.set_promised_id(promised_ballot_.GetProposalId());
  state.set_promised_node_id(promised_ballot_.GetNodeId());
  state.set_accepted_id(accepted_ballot_.GetProposalId());
  state.set_accepted_node_id(accepted_ballot_.GetNodeId());
  state.set_accepted_value(accepted_value_);
  WriteOptions options;
  options.sync = config_->LogSync();
  if (options.sync) {
    ++log_sync_count_;
    if (log_sync_count_ > config_->SyncInterval()) {
      log_sync_count_ = 0;
    } else {
      options.sync = false;
    }
  }

  std::string value;
  state.SerializeToString(&value);
  int ret = config_->GetDB()->Put(options, instance_id, value);
  return ret;
}

}  // namespace paxos
}  // namespace voyager
