// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/membership_machine.h"
#include "paxos/config.h"
#include "proto/paxos.pb.h"
#include "skywalker/logging.h"
#include "skywalker/file.h"

namespace skywalker {

MembershipMachine::MembershipMachine(Config* config,
                                     const GroupOptions& options)
    : config_(config),
      membership_(new Membership()) {
  set_machine_id(1);
  MemberMessage member;
  for (auto& i : options.membership) {
    member.set_id(i.id);
    member.set_host(i.host);
    member.set_port(i.port);
    member.set_context(i.context);
    membership_->mutable_members()->insert({member.id(), std::move(member)});
  }
}

bool MembershipMachine::Recover(uint32_t group_id, uint64_t instance_id,
                                const std::string& dir) {
  std::string data;
  if (!StateMachine::ReadCheckpoint(
      group_id, instance_id, dir + "/" + kMembershipCheckpoint, membership_.get())) {
    return false;
  }
  LOG_INFO("Group %u - instance %llu membership recover success.",
            group_id, (unsigned long long)instance_id);
  return true;
}

bool MembershipMachine::MakeCheckpoint(uint32_t group_id,
                                       uint64_t instance_id,
                                       const std::string& dir,
                                       const FinishCheckpointCallback& cb) {
  config_->GetCleanLoop()->QueueInLoop(
      [this, group_id, instance_id, dir, cb, membership = *membership_]() {
    bool b = StateMachine::WriteCheckpoint(
        group_id, instance_id, dir + "/" + kMembershipCheckpoint, membership);
    if (b) {
      LOG_INFO("Group %u - instance %llu membership make checkpoint success.",
                group_id, (unsigned long long)instance_id);
    }
    cb(machine_id(), group_id, instance_id, b);
  });
  return true;
}

bool MembershipMachine::GetCheckpoint(uint32_t group_id, uint64_t instance_id,
                                      const std::string& dir,
                                      std::vector<std::string>* files) {
  files->push_back(kMembershipCheckpoint);
  return true;
}

bool MembershipMachine::Execute(uint32_t group_id, uint64_t instance_id,
                                const std::string& value, void* /* context */) {
  if (instance_id <= membership_->version()) {
    LOG_ERROR("Group %u - instance(id=%llu) < membership version(%llu)",
              group_id, (unsigned long long)instance_id,
              (unsigned long long) membership_->version());
    return false;
  }
  MemberChangeMessage temp;
  if (temp.ParseFromString(value)) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      // Copy on write
      if (!membership_.unique()) {
        std::shared_ptr<Membership> new_membership(new Membership(*membership_));
        membership_.swap(new_membership);
      }
      assert(membership_.unique());

      for (int i = 0; i < temp.type_size(); ++i) {
        const MemberMessage& member = temp.member(i);
        if (temp.type(i) == MEMBER_ADD) {
          (*(membership_->mutable_members()))[member.id()] = member;
        } else if (temp.type(i) == MEMBER_REMOVE) {
          membership_->mutable_members()->erase(member.id());
        }
      }
      membership_->set_version(instance_id);
    }
    if (cb_) {
      cb_(group_id);
    }
    return true;
  } else {
    LOG_ERROR("Group %u - instance %llu membership parse from string failed.",
              config_->GetGroupId(), (unsigned long long)instance_id);
  }
  return false;
}

std::shared_ptr<Membership> MembershipMachine::GetMembership() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return membership_;
}

}  // namespace skywalker
