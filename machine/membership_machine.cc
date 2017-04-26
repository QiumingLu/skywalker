// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/membership_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"
#include "proto/paxos.pb.h"
#include "util/mutexlock.h"
#include "paxos/node_util.h"

namespace skywalker {

MembershipMachine::MembershipMachine(Config* config)
    : config_(config),
      db_(config->GetDB()),
      has_sync_membership_(false) {
  set_machine_id(0);
}

void MembershipMachine::Recover() {
  Membership m;
  int ret = db_->GetMembership(&m);
  if (ret == 0) {
    has_sync_membership_ = true;
    membership_ = m;
    config_->SetMembership(membership_);
  } else {
    membership_ = config_->GetMembership();
  }
}

bool MembershipMachine::Execute(uint32_t group_id, uint64_t instance_id,
                                const std::string& value,
                                MachineContext* /* context */) {
  Membership m;
  if (m.ParseFromString(value)) {
    if (instance_id < membership_.version()) {
      return true;
    }
    m.set_version(instance_id);
    int ret = db_->SetMembership(m);
    if (ret == 0) {
      MutexLock lock(&mutex_);
      membership_ = m;
      // copy to config.
      config_->SetMembership(m);
      if (!has_sync_membership_) {
        has_sync_membership_ = true;
      }
      return true;
    } else {
      SWLog(ERROR, "Update membership failed.\n");
    }
  } else {
    SWLog(ERROR, " Membership ParseFromString failed.\n");
  }
  return false;
}

const Membership& MembershipMachine::GetMembership() const {
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

}  // namespace skywalker
