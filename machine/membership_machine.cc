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
    (*(membership_->mutable_members()))[member.id()] = member;
  }
}

bool MembershipMachine::Recover(uint32_t group_id, uint64_t instance_id,
                                const std::string& dir,
                                const std::vector<std::string>& files) {
  if (dir.empty() || files.empty()) {
    return true;
  }
  std::string data;
  Status status = ReadFileToString(FileManager::Instance(), files[0], &data);
  if (!status.ok()) {
    LOG_ERROR("Group %u - instance %llu membership read failed %s.",
              config_->GetGroupId(), instance_id, status.ToString().c_str());
    return false;
  }
  if (membership_->ParseFromString(data)) {
    return true;
  } else {
    LOG_ERROR("Group %u - instance %llu membership parse failed.",
              config_->GetGroupId(), instance_id); 
  }
  return false;
}

bool MembershipMachine::Execute(uint32_t group_id, uint64_t instance_id,
                                const std::string& value, void* /* context */) {
  MemberChangeMessage temp;
  if (temp.ParseFromString(value)) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (instance_id <= membership_->version()) {
      return false;
    }

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
    mutex_.unlock();
    if (cb_) {
      cb_(group_id);
    }
    return true;
  } else {
    LOG_ERROR("Group %u - instance %llu membership parse from string failed.",
              config_->GetGroupId(), instance_id);
  }
  return false;
}

std::shared_ptr<Membership> MembershipMachine::GetMembership() const {
  std::unique_lock<std::mutex> lock(mutex_);
  return membership_;
}

bool MembershipMachine::MakeCheckpoint(uint32_t group_id,
                                       uint64_t instance_id,
                                       const std::string& dir) {
  Status status = WriteStringToFileSync(
      FileManager::Instance(), membership_->SerializeAsString(), dir + "/membershipdb");
  if (!status.ok()) {
    LOG_ERROR("Group %u - instance %llu membership write failed %s.",
              config_->GetGroupId(), instance_id, status.ToString().c_str());
    return false;
  }
  return true;
}

}  // namespace skywalker
