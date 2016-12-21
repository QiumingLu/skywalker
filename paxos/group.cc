#include "paxos/group.h"
#include "skywalker/logging.h"
#include "util/mutexlock.h"
#include "paxos/node_util.h"

namespace skywalker {

Group::Group(uint32_t group_id, uint64_t node_id,
             const Options& options, Network* network)
    : config_(group_id, node_id, options, network),
      instance_(&config_),
      loop_(config_.GetLoop()),
      machine_(config_.GetMachine()),
      mutex_(),
      cond_(&mutex_) {
}

bool Group::Start() {
  bool ret = config_.Init();
  if (ret) {
    ret = instance_.Init();
    if (ret) {
      instance_.SetProposeCompleteCallback(
          std::bind(&Group::ProposeComplete, this,
                    std::placeholders::_1, std::placeholders::_2));
      instance_.AddMachine(machine_);
    }
  }
  return ret;
}

void Group::SyncData() {
  int i = 0;
  instance_.SyncData();
  while (true) {
    if (i++ > 3) {
      instance_.SyncData();
    }
    const Membership& m = config_.GetMembership();
    if (!config_.HasSyncMembership()) {
      uint64_t instance_id;
      std::string s;
      m.SerializeToString(&s);
      int ret = OnPropose(s, &instance_id, machine_->GetMachineId());
      if (ret == 0 || ret == 1) {
        break;
      }
    } else {
      break;
    }
  }
}

int Group::OnPropose(const Slice& value,
                     uint64_t* instance_id,
                     int machine_id) {
  // FIXME
  MutexLock lock(&mutex_);
  propose_end_ = false;
  instance_id_ = 0;
  result_ = -1;
  loop_->QueueInLoop([value, machine_id, this]() {
    instance_.OnPropose(value, machine_id);
  });

  while (!propose_end_) {
    cond_.Wait();
  }

  *instance_id = instance_id_;
  return result_;
}

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  loop_->QueueInLoop([c, this]() {
    instance_.OnReceiveContent(c);
  });
}

void Group::ProposeComplete(int result, uint64_t instance_id) {
  // FIXME
  MutexLock lock(&mutex_);
  if (result == 0) {
    SWLog(INFO, "Group::OnReceivePropose - propose new value success.\n");
  } else if (result == 1) {
    SWLog(INFO,
          "Group::OnReceivePropose - propose new value failed! "
          "Because another value has been chosen.\n");
  } else if (result == 2) {
    SWLog(INFO, "Group::OnReceivePropose - machines execute failed!\n");
  } else {
    SWLog(INFO, "Group::OnReceivePropose - propose new value timeout.\n");
  }
  result_ = result;
  instance_id_ = instance_id;
  propose_end_ = true;
  cond_.Signal();
}

void Group::AddMachine(StateMachine* machine) {
  instance_.AddMachine(machine);
}

void Group::RemoveMachine(StateMachine* machine) {
  instance_.RemoveMachine(machine);
}

int Group::AddMember(const IpPort& ip) {
  if (!config_.HasSyncMembership()) {
    return 2;
  }
  bool res = false;
  uint64_t node_id(MakeNodeId(ip));
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
    uint64_t instance_id = 0;
    return OnPropose(s, &instance_id, machine_->GetMachineId());
  } else {
    return 1;
  }
}

int Group::RemoveMember(const IpPort& ip) {
  if (!config_.HasSyncMembership()) {
    return 2;
  }
  bool res = false;
  uint64_t node_id(MakeNodeId(ip));
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
    uint64_t instance_id = 0;
    return OnPropose(s, &instance_id, machine_->GetMachineId());
  } else {
    return 1;
  }
}

int Group::ReplaceMember(const IpPort& new_i, const IpPort& old_i) {
  if (!config_.HasSyncMembership()) {
    return 2;
  }

  bool new_res = false;
  bool old_res = false;
  uint64_t new_node_id(MakeNodeId(new_i));
  uint64_t old_node_id(MakeNodeId(old_i));
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
    return 1;
  } else {
    std::string s;
    m.SerializeToString(&s);
    uint64_t instance_id = 0;
    return OnPropose(s, &instance_id, machine_->GetMachineId());
  }
}

}  // namespace skywalker
