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
      has_sync_membership_(false) {
  set_machine_id(0);
}

void MembershipMachine::Recover() {
  Membership m;
  int ret = config_->GetDB()->GetMembership(&m);
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
    int ret = config_->GetDB()->SetMembership(m);
    if (ret == 0) {
      SetMembership(m);
      return true;
    } else {
      LOG_ERROR("Group %u - update membership failed.", config_->GetGroupId());
    }
  } else {
    LOG_ERROR("Group %u - membership parse from string failed.",
              config_->GetGroupId());
  }
  return false;
}

std::string MembershipMachine::GetString() const {
  std::string s;
  membership_.SerializeToString(&s);
  return s;
}

void MembershipMachine::SetString(const std::string& s) {
  Membership membership;
  membership.ParseFromString(s);
  if (membership.version() > membership_.version()) {
    SetMembership(membership);
  }
}

const Membership& MembershipMachine::GetMembership() const {
  return membership_;
}

void MembershipMachine::SetMembership(const Membership& m) {
  MutexLock lock(&mutex_);
  membership_ = m;
  config_->SetMembership(m);
  if (!has_sync_membership_) {
    has_sync_membership_ = true;
  }
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
