#include "paxos/group.h"
#include "util/mutexlock.h"
#include "paxos/node_util.h"
#include <unistd.h>

namespace skywalker {

Group::Group(uint32_t group_id, uint64_t node_id,
             const Options& options, Network* network)
    : config_(group_id, node_id, options, network),
      instance_(&config_),
      loop_(config_.GetLoop()),
      machine_(config_.GetMachine()),
      mutex_(),
      cond_(&mutex_),
      last_finish_(true),
      propose_end_(false) {
  instance_.SetProposeCompleteCallback(
      std::bind(&Group::ProposeComplete, this,
                std::placeholders::_1, std::placeholders::_2));
  instance_.AddMachine(machine_);
}

bool Group::Start() {
  bool ret = config_.Init();
  if (ret) {
    return instance_.Init();
  }
  return ret;
}

void Group::SyncMembership() {
  int i = 0;
  while (true) {
    if (i++ > 3) {
      instance_.SyncData();
    }
    MutexLock lock(&mutex_);
    propose_end_ = false;
    result_ = Status::OK();
    loop_->QueueInLoop([this]() {
      SyncMembershipInLoop();
    });
    while (!propose_end_) {
      cond_.Wait();
    }
    if (result_.ok() || result_.IsConflict()) {
      break;
    }
    usleep(30*1000);
  }
}

void Group::SyncMembershipInLoop() {
  if (!config_.HasSyncMembership()) {
    const Membership& m = config_.GetMembership();
    std::string s;
    m.SerializeToString(&s);
    instance_.OnPropose(s, machine_->GetMachineId());
  } else {
     propose_end_ = true;
     result_ = Status::OK();
     cond_.Signal();
   }
}

Status Group::OnPropose(const Slice& value,
                        uint64_t* instance_id,
                        int machine_id) {
  // FIXME:releasize non-blocking by using a deque or another...?
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

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  loop_->QueueInLoop([c, this]() {
    instance_.OnReceiveContent(c);
  });
}

Status Group::AddMember(const IpPort& ip) {
  uint64_t node_id(MakeNodeId(ip));
  MutexLock lock(&mutex_);
  while (!last_finish_) {
    cond_.Wait();
  }
  last_finish_ = false;
  propose_end_ = false;
  loop_->QueueInLoop([node_id, this] {
    AddMemberInLoop(node_id);
  });
  while (!propose_end_) {
    cond_.Wait();
  }
  last_finish_ = true;
  cond_.Signal();
  return result_;
}

void Group::AddMemberInLoop(uint64_t node_id) {
  if (!config_.HasSyncMembership()) {
    result_ = Status::Unavailable("Membership hasn't been synchronized.");
  } else {
    bool res = false;
    const Membership& temp = config_.GetMembership();

    for (int i = 0; i < temp.node_id_size(); ++i) {
      if (node_id == temp.node_id(i)) {
        res = true;
        break;
      }
    }
    if (!res) {
      Membership m(temp);
      m.add_node_id(node_id);
      std::string s;
      m.SerializeToString(&s);
      instance_.OnPropose(s, machine_->GetMachineId(), false);
    } else {
      result_ = Status::AlreadyExists(Slice());
    }
  }
  if (!result_.ok()) {
    propose_end_ = true;
    cond_.SignalAll();
  }
}

Status Group::RemoveMember(const IpPort& ip) {
  uint64_t node_id(MakeNodeId(ip));
  MutexLock lock(&mutex_);
  while (!last_finish_) {
    cond_.Wait();
  }
  last_finish_ = false;
  propose_end_ = false;
  result_ = Status::OK();
  loop_->QueueInLoop([node_id, this] {
    RemoveMemberInLoop(node_id);
  });
  while (!propose_end_) {
    cond_.Wait();
  }
  last_finish_ = true;
  cond_.Signal();
  return result_;
}

void Group::RemoveMemberInLoop(uint64_t node_id) {
  if (!config_.HasSyncMembership()) {
    result_ = Status::Unavailable("Membership hasn't been synchronized.");
  } else {
    bool res = false;
    const Membership& temp = config_.GetMembership();
    Membership m;
    for (int i = 0; i < temp.node_id_size(); ++i) {
      if (node_id == temp.node_id(i)) {
        res = true;
      } else {
        m.add_node_id(temp.node_id(i));
      }
    }

    if (res) {
      std::string s;
      m.SerializeToString(&s);
      instance_.OnPropose(s, machine_->GetMachineId(), false);
    } else {
      result_ = Status::NotFound(Slice());
    }
  }
  if (!result_.ok()) {
    propose_end_ = true;
    cond_.SignalAll();
  }
}

Status Group::ReplaceMember(const IpPort& new_i, const IpPort& old_i) {
  uint64_t new_node_id(MakeNodeId(new_i));
  uint64_t old_node_id(MakeNodeId(old_i));
  MutexLock lock(&mutex_);
  while (!last_finish_) {
    cond_.Wait();
  }
  last_finish_ = false;
  propose_end_ = false;
  result_ = Status::OK();
  loop_->QueueInLoop([new_node_id, old_node_id, this] {
    ReplaceMemberInLoop(new_node_id, old_node_id);
  });
  while(!propose_end_) {
    cond_.Wait();
  }
  last_finish_ = false;
  cond_.Signal();
  return result_;
}

void Group::ReplaceMemberInLoop(uint64_t new_node_id,
                                uint64_t old_node_id) {
  if (!config_.HasSyncMembership()) {
    result_ = Status::Unavailable("Membership hasn't been synchronized.");
  } else {
    bool new_res = false;
    bool old_res = false;
    const Membership& temp = config_.GetMembership();
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
      result_ = Status::AlreadyExists(Slice());
    } else {
      if (!new_res) {
        m.add_node_id(new_node_id);
      }
      std::string s;
      m.SerializeToString(&s);
      instance_.OnPropose(s, machine_->GetMachineId(), false);
    }
  }
  if (!result_.ok()) {
    propose_end_ = true;
    cond_.SignalAll();
  }
}

void Group::ProposeComplete(Status&& result, uint64_t instance_id) {
  result_ = std::move(result);
  instance_id_ = instance_id;
  propose_end_ = true;
  cond_.SignalAll();
}

void Group::AddMachine(StateMachine* machine) {
  instance_.AddMachine(machine);
}

void Group::RemoveMachine(StateMachine* machine) {
  instance_.RemoveMachine(machine);
}

}  // namespace skywalker
