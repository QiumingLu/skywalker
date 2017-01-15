#include "paxos/acceptor.h"
#include "paxos/config.h"
#include "paxos/instance.h"
#include "skywalker/logging.h"

namespace skywalker {

Acceptor::Acceptor(Config* config, Instance* instance)
    : config_(config),
      instance_(instance),
      messager_(config->GetMessager()),
      instance_id_(0),
      log_sync_count_(0) {
}

bool Acceptor::Init(uint64_t* instance_id) {
  if (ReadFromDB()) {
    *instance_id = instance_id_;
    return true;
  }
  return false;
}

void Acceptor::OnPrepare(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
    PaxosMessage* reply_msg = new PaxosMessage();
    reply_msg->set_type(PREPARE_REPLY);
    reply_msg->set_node_id(config_->GetNodeId());
    reply_msg->set_instance_id(instance_id_);
    reply_msg->set_proposal_id(msg.proposal_id());

    BallotNumber b(msg.proposal_id(), msg.node_id());

    if (b >= promised_ballot_) {
      promised_ballot_ =  b;
      if (accepted_ballot_.GetProposalId() > 0) {
        reply_msg->set_pre_accepted_id(accepted_ballot_.GetProposalId());
        reply_msg->set_pre_accepted_node_id(accepted_ballot_.GetNodeId());
        reply_msg->set_allocated_value(new PaxosValue(accepted_value_));
      }
      if (!WriteToDB()) {
        SWLog(ERROR, "Acceptor::OnPrepare - "
              "write instance_id=%" PRIu64" to db failed.\n", instance_id_);
      }
    } else {
      reply_msg->set_rejected_id(promised_ballot_.GetProposalId());
    }

    if (msg.node_id() == config_->GetNodeId()) {
      instance_->OnPaxosMessage(*reply_msg);
      delete reply_msg;
    } else {
      messager_->SendMessage(msg.node_id(),
                             messager_->PackMessage(reply_msg));
    }
  } else if (msg.instance_id() == instance_id_ + 1) {
    NewChosenValue(msg);
  }
}

void Acceptor::OnAccpet(const PaxosMessage& msg) {
  if (msg.instance_id() == instance_id_) {
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
      if (!WriteToDB()) {
        SWLog(ERROR, "Acceptor::OnAccpet - "
              "write instance_id=%" PRIu64" to db failed.\n", instance_id_);
      }
    } else {
      reply_msg->set_rejected_id(promised_ballot_.GetProposalId());
    }

    if (msg.node_id() == config_->GetNodeId()) {
      instance_->OnPaxosMessage(*reply_msg);
      delete reply_msg;
    } else {
      messager_->SendMessage(msg.node_id(),
                             messager_->PackMessage(reply_msg));
    }
  } else if (msg.instance_id() == instance_id_ + 1) {
    NewChosenValue(msg);
  }
}

// Don't reset the promised_ballot_ here so that
// the proposer can reduce to call prepare function in sometimes.
void Acceptor::NextInstance() {
  ++instance_id_;
  accepted_ballot_.Reset();
  accepted_value_.Clear();
}

void Acceptor::NewChosenValue(const PaxosMessage& msg) {
  PaxosMessage new_msg;
  new_msg.set_type(NEW_CHOSEN_VALUE);
  new_msg.set_node_id(msg.node_id());
  new_msg.set_instance_id(instance_id_);
  new_msg.set_proposal_id(msg.proposal_id());
  instance_->OnPaxosMessage(new_msg);
}

bool Acceptor::ReadFromDB() {
  int res = config_->GetDB()->GetMaxInstanceId(&instance_id_);
  if (res == 1) {
    return true;
  } else if (res == -1) {
    return false;
  }

  std::string s;
  res = config_->GetDB()->Get(instance_id_, &s);
  if (res !=  0) {
    return false;
  }

  AcceptorState state;
  state.ParseFromString(s);
  promised_ballot_.SetProposalId(state.promised_id());
  promised_ballot_.SetNodeId(state.promised_node_id());
  accepted_ballot_.SetProposalId(state.accepted_id());
  accepted_ballot_.SetNodeId(state.accepted_node_id());
  accepted_value_ = state.accepted_value();
  return true;
}

bool Acceptor::WriteToDB() {
  AcceptorState state;
  state.set_instance_id(instance_id_);
  state.set_promised_id(promised_ballot_.GetProposalId());
  state.set_promised_node_id(promised_ballot_.GetNodeId());
  state.set_accepted_id(accepted_ballot_.GetProposalId());
  state.set_accepted_node_id(accepted_ballot_.GetNodeId());
  state.set_allocated_accepted_value(new PaxosValue(accepted_value_));
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

  std::string s;
  state.SerializeToString(&s);
  int ret = config_->GetDB()->Put(options, instance_id_, s);
  if (ret == 0) {
    return true;
  } else {
    return false;
  }
}

}  // namespace skywalker
