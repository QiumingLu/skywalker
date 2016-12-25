#include "machine/membership_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"
#include "paxos/paxos.pb.h"
#include "util/mutexlock.h"
#include "paxos/node_util.h"

namespace skywalker {

MembershipMachine::MembershipMachine(const Options& options, Config* config)
    : config_(config),
      db_(config->GetDB()),
      has_sync_membership_(false) {
  membership_.set_version(0);
  for (auto i : options.membership) {
    membership_.add_node_id(MakeNodeId(i));
  }
}

void MembershipMachine::Recover() {
  Membership m;
  int ret = db_->GetMembership(&m);
  if (ret == 0) {
    has_sync_membership_ = true;
    membership_ = m;
  }
  config_->SetMembership(membership_);
}

bool MembershipMachine::Execute(uint32_t group_id, uint64_t instance_id,
                                const std::string& value) {
  Membership m;
  if (m.ParseFromString(value)) {
    int ret = db_->SetMembership(m);
    if (ret == 0) {
      m.set_version(instance_id);
      MutexLock lock(&mutex_);
      membership_ = m;
      config_->SetMembership(m);
      if (!has_sync_membership_) {
        has_sync_membership_ = true;
      }
      return true;
    } else {
      SWLog(ERROR, "MembershipMachine::Execute - update membership failed.\n");
    }
  } else {
    SWLog(ERROR, "MembershipMachine::Execute - m.ParseFromString failed.\n");
  }
  return false;
}

Membership MembershipMachine::GetMembership() const {
  MutexLock lock(&mutex_);
  return membership_;
}

void MembershipMachine::GetMembership(std::vector<IpPort>* result) const {
  MutexLock lock(&mutex_);
  for (int i = 0; i < membership_.node_id_size(); ++i) {
    IpPort ip;
    ParseNodeId(membership_.node_id(i), &ip);
    result->push_back(ip);
  }
}

bool MembershipMachine::HasSyncMembership() const {
  MutexLock lock(&mutex_);
  return has_sync_membership_;
}


bool MembershipMachine::IsValidNodeId(uint64_t node_id) const {
  MutexLock lock(&mutex_);
  for (int i = 0; i < membership_.node_id_size(); ++i) {
    if (node_id == membership_.node_id(i)) {
      return true;
    }
  }
  return false;
}

}  // namespace skywalker
