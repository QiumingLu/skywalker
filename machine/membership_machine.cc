// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/membership_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"
#include "proto/paxos.pb.h"
#include "util/mutexlock.h"

namespace skywalker {

MembershipMachine::MembershipMachine(Config* config, const GroupOptions& options)
    : config_(config),
      has_sync_membership_(false),
      membership_(new Membership()) {
  set_machine_id(0);
  MemberMessage member;
  for (auto& i : options.membership) {
    member.set_id(i.id);
    member.set_ip(i.ip);
    member.set_port(i.port);
    member.set_context(i.context);
    (*(membership_->mutable_members()))[member.id()] = member;
  }
}

void MembershipMachine::Recover() {
  Membership* temp = new Membership();
  int ret = config_->GetDB()->GetMembership(temp);
  if (ret == 0) {
    has_sync_membership_ = true;
    membership_.reset(temp);
  } else {
    delete temp;
  }
}

bool MembershipMachine::Execute(uint32_t group_id, uint64_t instance_id,
                                const std::string& value,
                                MachineContext* /* context */) {
  MemberChangeMessage temp;
  if (temp.ParseFromString(value)) {
    MutexLock lock(&mutex_);
    if (instance_id < membership_->version()) {
      return true;
    }

    // Copy on write
    if (!membership_.unique()) {
      std::shared_ptr<Membership> new_membership(new Membership(*membership_));
      membership_.swap(new_membership);
    } else if (!has_sync_membership_) {
      membership_.reset(new Membership());
    }
    if (!has_sync_membership_) {
      has_sync_membership_= true;
    }
    assert(membership_.unique());

    for (int i = 0; i < temp.type_size(); ++i) {
      const MemberMessage& member = temp.member(i);
      if (temp.type(i) == MEMBER_ADD) {
        (*(membership_->mutable_members()))[member.id()] = member;
      } else if (temp.type(i) == MEMBER_REMOVE){
        membership_->mutable_members()->erase(member.id());
      }
    }
    membership_->set_version(instance_id);

    int ret = config_->GetDB()->SetMembership(*membership_);
    if (ret == 0) {
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
  MutexLock lock(&mutex_);
  std::string s;
  membership_->SerializeToString(&s);
  return s;
}

void MembershipMachine::SetString(const std::string& s) {
  Membership* temp = new Membership();
  temp->ParseFromString(s);

  MutexLock lock(&mutex_);
  if (temp->version() > membership_->version()) {
    membership_.reset(temp);
  } else {
    delete temp;
  }
}

std::shared_ptr<Membership> MembershipMachine::GetMembership() const {
  MutexLock lock(&mutex_);
  return membership_;
}

bool MembershipMachine::HasSyncMembership() const {
  return has_sync_membership_;
}

}  // namespace skywalker
