#include "paxos/group.h"
#include "util/mutexlock.h"
#include "paxos/node_util.h"
#include <unistd.h>

namespace skywalker {

Group::Group(uint32_t group_id, uint64_t node_id,
             const Options& options, Network* network)
    : node_id_(node_id),
      config_(group_id, node_id, options, network),
      instance_(&config_),
      loop_(config_.GetLoop()),
      bg_loop_(config_.GetBGLoop()),
      lease_timeout_(5000 * 1000),
      retrie_master_(false),
      membership_machine_(options, &config_),
      master_machine_(&config_),
      mutex_(),
      cond_(&mutex_),
      last_finish_(true),
      propose_end_(false) {

  instance_.SetProposeCompleteCallback(
      std::bind(&Group::ProposeComplete, this,
                std::placeholders::_1, std::placeholders::_2));
  instance_.AddMachine(&membership_machine_);
  instance_.AddMachine(&master_machine_);
}

bool Group::Start() {
  if(config_.Init() && instance_.Init()) {
    membership_machine_.Recover();
    master_machine_.Recover();
    return true;
  } else {
    return false;
  }
}

void Group::SyncMembership() {
  int i = 0;
  while (true) {
    if (i++ > 3) {
      instance_.SyncData();
    }
    if (!membership_machine_.HasSyncMembership()) {
      Membership m(membership_machine_.GetMembership());
      std::string s;
      m.SerializeToString(&s);
      uint64_t id;
      OnPropose(s, &id, membership_machine_.GetMachineId());
    }

    if (result_.ok() || result_.IsConflict()) {
      break;
    }
    usleep(30*1000);
  }
}

void Group::SyncMaster() {
  bg_loop_->QueueInLoop([this]() {
    TryBeMaster();
  });
}

void Group::TryBeMaster() {
  MasterState state(master_machine_.GetMasterState());
  if (state.lease_time() <= NowMicros() ||
      (state.node_id() == node_id_ && !retrie_master_)) {
    state.set_node_id(node_id_);
    state.set_lease_time(lease_timeout_);
    std::string s;
    state.SerializeToString(&s);
    uint64_t i;
    OnPropose(s, &i, master_machine_.GetMachineId());
    state = master_machine_.GetMasterState();
  }
  if (retrie_master_) {
    retrie_master_ = false;
  }
  if (state.node_id() == node_id_) {
    next_try_be_master_time_ = state.lease_time() - 200 * 1000;
  } else {
    next_try_be_master_time_ = state.lease_time();
  }
  bg_loop_->RunAt((next_try_be_master_time_/1000), [this]() {
    TryBeMaster();
  });
}

Status Group::OnPropose(const Slice& value,
                        uint64_t* instance_id,
                        int machine_id,
                        bool check_valid) {
  if (check_valid && !membership_machine_.IsValidNodeId(node_id_)) {
    Slice msg("this node is not in the membership, please add it firstly.");
    return Status::InvalidNode(msg);
  }
  MutexLock lock(&mutex_);
  while (!last_finish_) {
    cond_.Wait();
  }
  last_finish_ = false;
  propose_end_ = false;
  instance_id_ = 0;
  loop_->QueueInLoop([value, machine_id, this]() {
    instance_.OnPropose(value, machine_id);
  });

  while (!propose_end_) {
    cond_.Wait();
  }
  *instance_id = instance_id_;
  last_finish_ = true;
  cond_.Signal();
  return result_;
}

void Group::ProposeComplete(Status&& result, uint64_t instance_id) {
  result_ = std::move(result);
  instance_id_ = instance_id;
  propose_end_ = true;
  cond_.SignalAll();
}

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  loop_->QueueInLoop([c, this]() {
    instance_.OnReceiveContent(c);
  });
}

Status Group::AddMember(const IpPort& ip) {
  if (!membership_machine_.HasSyncMembership()) {
    return Status::Unavailable("Membership hasn't been synchronized.");
  } else {
    uint64_t node_id(MakeNodeId(ip));
    bool res = false;
    Membership m(membership_machine_.GetMembership());
    for (int i = 0; i < m.node_id_size(); ++i) {
      if (node_id == m.node_id(i)) {
        res = true;
        break;
      }
    }
    if (res) {
      return Status::AlreadyExists(Slice());
    } else {
      m.add_node_id(node_id);
      std::string s;
      m.SerializeToString(&s);
      uint64_t id;
      return OnPropose(s, &id, membership_machine_.GetMachineId(), false);
    }
  }
}

Status Group::RemoveMember(const IpPort& ip) {
  if (!membership_machine_.HasSyncMembership()) {
    return Status::Unavailable("Membership hasn't been synchronized.");
  } else {
    uint64_t node_id(MakeNodeId(ip));
    bool res = false;
    Membership temp(membership_machine_.GetMembership());
    Membership m;
    for (int i = 0; i < temp.node_id_size(); ++i) {
      if (node_id == temp.node_id(i)) {
        res = true;
      } else {
        m.add_node_id(temp.node_id(i));
      }
    }

    if (!res) {
      return Status::NotFound(Slice());
    } else {
      std::string s;
      m.SerializeToString(&s);
      uint64_t id;
      return OnPropose(s, &id, membership_machine_.GetMachineId(), false);
    }
  }
}

Status Group::ReplaceMember(const IpPort& new_i, const IpPort& old_i) {
  if (!membership_machine_.HasSyncMembership()) {
    return Status::Unavailable("Membership hasn't been synchronized.");
  } else {
    uint64_t new_node_id(MakeNodeId(new_i));
    uint64_t old_node_id(MakeNodeId(old_i));
    bool new_res = false;
    bool old_res = false;
    Membership temp(membership_machine_.GetMembership());
    Membership m;
    for (int i = 0; i < temp.node_id_size(); ++i) {
      if (temp.node_id(i) == new_node_id) {
        new_res = true;
      }
      if (temp.node_id(i) == old_node_id) {
        old_res = true;
      } else {
        m.add_node_id(temp.node_id(i));
      }
    }

    if (new_res &&  (!old_res)) {
      return Status::AlreadyExists(Slice());
    } else {
      if (!new_res) {
        m.add_node_id(new_node_id);
      }
      std::string s;
      m.SerializeToString(&s);
      uint64_t id;
      return OnPropose(s, &id, membership_machine_.GetMachineId(), false);
    }
  }
}

void Group::GetMembership(std::vector<IpPort>* result) const {
  membership_machine_.GetMembership(result);
}

void Group::AddMachine(StateMachine* machine) {
  instance_.AddMachine(machine);
}

void Group::RemoveMachine(StateMachine* machine) {
  instance_.RemoveMachine(machine);
}

void Group::SetMasterLeaseTime(uint64_t micros) {
  bg_loop_->QueueInLoop([micros, this]() {
    if (micros < (1000*1000)) {
      lease_timeout_ = 1000 * 1000;
    } else {
      lease_timeout_ = micros;
    }
  });
}

void Group::GetMaster(IpPort* i) const {
  master_machine_.GetMaster(i);
}

bool Group::IsMaster() const {
  return master_machine_.IsMaster();
}

void Group::RetireMaster() {
  bg_loop_->QueueInLoop([this]() {
    retrie_master_ = true;
  });
}

}  // namespace skywalker
