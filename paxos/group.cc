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
      bg_loop_(),
      lease_timeout_(10 * 1000 * 1000),
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
  instance_.SyncData();
  int i = 0;
  while (true) {
    if (i++ > 3) {
      instance_.SyncData();
    }
    if (!membership_machine_.HasSyncMembership()) {
      MachineContext* context(
          new MachineContext(membership_machine_.GetMachineId()));
      Func f(std::bind(&Group::SyncMembershipInLoop, this, context));
      Status status = NewPropose(f);
      delete context;
      if (status.ok() || status.IsConflict()) {
        break;
      }
    } else {
      break;
    }
    usleep(30*1000);
  }
}

void Group::SyncMembershipInLoop(MachineContext* context) {
  if (!membership_machine_.HasSyncMembership()) {
    const Membership& m(membership_machine_.GetMembership());
    std::string s;
    m.SerializeToString(&s);
    instance_.OnPropose(s, context);
  } else {
    ProposeComplete(Status::OK(), instance_.GetInstanceId());
  }
}

void Group::SyncMaster() {
  bg_loop_.Loop();
  bg_loop_.QueueInLoop([this]() {
    TryBeMaster();
  });
}

void Group::TryBeMaster() {
  MasterState state(master_machine_.GetMasterState());
  uint64_t now = NowMicros();
  if (state.lease_time() <= now ||
      (state.node_id() == node_id_ && !retrie_master_)) {
    uint64_t lease_time = now + lease_timeout_;
    MachineContext *context(
        new MachineContext(master_machine_.GetMachineId(),
                           reinterpret_cast<void*>(&lease_time)));
    Func f(std::bind(&Group::TryBeMasterInLoop, this, context));
    Status status = NewPropose(f);
    if(status.ok() || status.IsConflict()) {
      state = master_machine_.GetMasterState();
    } else {
      state.set_lease_time(NowMicros() + lease_timeout_);
    }
    delete context;
  }
  if (retrie_master_) {
    retrie_master_ = false;
  }
  bg_loop_.RunAt(state.lease_time(), [this]() {
    TryBeMaster();
  });
}

void Group::TryBeMasterInLoop(MachineContext* context) {
  MasterState state(master_machine_.GetMasterState());
  if (state.lease_time() <= NowMicros()) {
    state.set_node_id(node_id_);
    state.set_lease_time(lease_timeout_);
    std::string s;
    state.SerializeToString(&s);
    instance_.OnPropose(s, context);
  } else {
    ProposeComplete(Status::Conflict(Slice()), instance_.GetInstanceId());
  }
}

Status Group::OnPropose(const Slice& value, int machine_id) {
  MachineContext context(machine_id);
  uint64_t instance_id;
  return OnPropose(value, &context, &instance_id);
}

Status Group::OnPropose(const Slice& value,
                        MachineContext* context,
                        uint64_t* instance_id) {
  Func f(std::bind(&Instance::OnPropose, &instance_, value, context));
  return NewPropose(f, instance_id);
}

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  loop_->QueueInLoop([c, this]() {
    instance_.OnReceiveContent(c);
  });
}

Status Group::AddMember(const IpPort& ip) {
  uint64_t node_id(MakeNodeId(ip));
  MachineContext* context(
      new MachineContext(membership_machine_.GetMachineId()));
  Func f(std::bind(&Group::AddMemberInLoop, this, node_id, context));
  Status status = NewPropose(f);
  delete context;
  return status;
}

void Group::AddMemberInLoop(uint64_t node_id,
                            MachineContext* context) {
  bool res = false;
  const Membership& temp(membership_machine_.GetMembership());
  for (int i = 0; i < temp.node_id_size(); ++i) {
    if (node_id == temp.node_id(i)) {
      res = true;
      break;
    }
  }
  if (res) {
    ProposeComplete(Status::OK(), instance_.GetInstanceId());
  } else {
    Membership m(temp);
    m.add_node_id(node_id);
    std::string s;
    m.SerializeToString(&s);
    instance_.OnPropose(s, context);
  }
}

Status Group::RemoveMember(const IpPort& ip) {
  uint64_t node_id(MakeNodeId(ip));
  MachineContext* context(
      new MachineContext(membership_machine_.GetMachineId()));
  Func f(std::bind(&Group::RemoveMemberInLoop, this, node_id, context));
  Status status = NewPropose(f);
  delete context;
  return status;
}

void Group::RemoveMemberInLoop(uint64_t node_id,
                               MachineContext* context) {
  bool res = false;
  const Membership& temp(membership_machine_.GetMembership());
  Membership m;
  for (int i = 0; i < temp.node_id_size(); ++i) {
    if (node_id == temp.node_id(i)) {
     res = true;
    } else {
      m.add_node_id(temp.node_id(i));
    }
  }

  if (!res) {
    ProposeComplete(Status::OK(), instance_.GetInstanceId());
  } else {
    std::string s;
    m.SerializeToString(&s);
    instance_.OnPropose(s, context);
  }
}

Status Group::ReplaceMember(const IpPort& new_i, const IpPort& old_i) {
  uint64_t i(MakeNodeId(new_i));
  uint64_t j(MakeNodeId(old_i));
  MachineContext* context(
      new MachineContext(membership_machine_.GetMachineId()));
  Func f(std::bind(&Group::ReplaceMemberInLoop, this, i, j, context));
  Status status = NewPropose(f);
  delete context;
  return status;
}

void Group::ReplaceMemberInLoop(uint64_t new_node_id, uint64_t old_node_id,
                                MachineContext* context) {
  bool new_res = false;
  bool old_res = false;
  const Membership& temp(membership_machine_.GetMembership());
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
    ProposeComplete(Status::OK(), instance_.GetInstanceId());
  } else {
    if (!new_res) {
      m.add_node_id(new_node_id);
    }
    std::string s;
    m.SerializeToString(&s);
    instance_.OnPropose(s, context);
  }
}

Status Group::NewPropose(const Func& f, uint64_t* instance_id) {
  MutexLock lock(&mutex_);
  while (!last_finish_) {
    cond_.Wait();
  }
  last_finish_ = false;
  propose_end_ = false;
  loop_->QueueInLoop([&f, this]() {
    f();
  });

  while (!propose_end_) {
    cond_.Wait();
  }
  if (instance_id != nullptr) {
    *instance_id = instance_id_;
  }
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
  bg_loop_.QueueInLoop([micros, this]() {
    if (micros < (5 *1000 * 1000)) {
      lease_timeout_ = 5 * 1000 * 1000;
    } else {
      lease_timeout_ = micros;
    }
  });
}

bool Group::GetMaster(IpPort* i, uint64_t* version) const {
  return master_machine_.GetMaster(i, version);
}

bool Group::IsMaster() const {
  return master_machine_.IsMaster();
}

void Group::RetireMaster() {
  bg_loop_.QueueInLoop([this]() {
    retrie_master_ = true;
  });
}

}  // namespace skywalker
