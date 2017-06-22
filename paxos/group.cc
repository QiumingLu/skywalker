// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/group.h"

#include <utility>

#include "skywalker/logging.h"
#include "util/mutexlock.h"
#include "util/timeops.h"

namespace skywalker {

Group::Group(uint64_t node_id, const GroupOptions& options, Network* network)
    : node_id_(node_id),
      config_(node_id, options, network),
      instance_(&config_),
      lease_timeout_(options.master_lease_time),
      retrie_master_(false),
      mutex_(),
      cond_(&mutex_),
      propose_end_(false),
      propose_queue_(100),
      schedule_(new Schedule(options.use_master)) {
  propose_cb_ = std::bind(&ProposeQueue::ProposeComplete, &propose_queue_,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3);
  instance_.SetProposeCompleteCallback(propose_cb_);

  membership_machine_ = config_.GetMembershipMachine();
  master_machine_ = config_.GetMasterMachine();
}

bool Group::Start() {
  if (config_.Init() && instance_.Recover()) {
    schedule_->Start();
    instance_.SetIOLoop(schedule_->IOLoop());
    instance_.SetLearnLoop(schedule_->LearnLoop());
    propose_queue_.SetIOLoop(schedule_->IOLoop());
    propose_queue_.SetCallbackLoop(schedule_->CallbackLoop());
    return true;
  }
  return false;
}

void Group::SyncMembership() {
  instance_.SyncData(true);
  int i = 0;
  while (true) {
    if (!membership_machine_->HasSyncMembership()) {
      NewPropose(std::bind(&Group::SyncMembershipInLoop, this));
    }
    if (membership_machine_->HasSyncMembership()) {
      break;
    }
    if (++i > 3) {
      instance_.SyncData(false);
      i = 0;
    }
    SleepForMicroseconds(500 * 1000);
  }
}

void Group::SyncMembershipInLoop() {
  if (!membership_machine_->HasSyncMembership()) {
    std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
    MemberChangeMessage message;
    for (auto& i : temp->members()) {
      *(message.add_member()) = i.second;
      message.add_type(MEMBER_ADD);
    }
    instance_.OnPropose(message.SerializeAsString(),
                        membership_machine_->machine_id());
  } else {
    propose_cb_(nullptr, Status::OK(), instance_.GetInstanceId());
  }
}

void Group::SyncMaster() {
  if (schedule_->MasterLoop() != nullptr) {
    TryBeMaster();
  }
}

void Group::TryBeMaster() {
  MasterState state(master_machine_->GetMasterState());
  uint64_t next = state.lease_time();
  if (state.lease_time() <= NowMicros() ||
      (state.node_id() == node_id_ && !retrie_master_)) {
    bool b = NewPropose(std::bind(&Group::TryBeMasterInLoop, this));
    next = 0;
    if (b) {
      state = master_machine_->GetMasterState();
      if (result_.ok()) {
        next = state.lease_time() - 100 * 1000;
      } else if (result_.IsConflict()) {
        next = state.lease_time();
      }
    }
    if (next == 0) {
      next = NowMicros() + lease_timeout_;
    }
  }
  if (retrie_master_) {
    retrie_master_ = false;
  }

  schedule_->MasterLoop()->RunAt(next, [this]() { TryBeMaster(); });
}

void Group::TryBeMasterInLoop() {
  MasterState state(master_machine_->GetMasterState());
  if (state.lease_time() <= NowMicros() || state.node_id() == node_id_) {
    state.set_node_id(node_id_);
    state.set_lease_time(lease_timeout_);
    instance_.OnPropose(state.SerializeAsString(),
                        master_machine_->machine_id());
  } else {
    propose_cb_(nullptr, Status::Conflict("Already has master"),
                instance_.GetInstanceId());
  }
}

bool Group::OnPropose(const std::string& value, int machine_id, void* context,
                      const ProposeCompleteCallback& cb) {
  return propose_queue_.Put(
      std::bind(&Instance::OnPropose, &instance_, value, machine_id, context),
      cb);
}

bool Group::OnPropose(const std::string& value, int machine_id, void* context,
                      ProposeCompleteCallback&& cb) {
  return propose_queue_.Put(
      std::bind(&Instance::OnPropose, &instance_, value, machine_id, context),
      std::move(cb));
}

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  schedule_->IOLoop()->QueueInLoop(
      [c, this]() { instance_.OnReceiveContent(c); });
}

bool Group::ChangeMember(const std::map<Member, bool>& value,
                         const ChangeMemberCompleteCallback& cb) {
  MemberMessage member;
  MemberChangeMessage change;
  for (auto i : value) {
    member.set_id(i.first.id);
    member.set_host(i.first.host);
    member.set_port(i.first.port);
    member.set_context(i.first.context);
    *(change.add_member()) = member;
    if (i.second) {
      change.add_type(MEMBER_ADD);
    } else {
      change.add_type(MEMBER_REMOVE);
    }
  }
  return OnPropose(change.SerializeAsString(),
                   membership_machine_->machine_id(), nullptr,
                   [cb](void*, const Status& s, uint64_t id) { cb(s, id); });
}

bool Group::NewPropose(ProposeHandler&& f) {
  MutexLock lock(&mutex_);
  propose_end_ = false;
  ProposeCompleteCallback cb =
      std::bind(&Group::ProposeComplete, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
  bool res = propose_queue_.Put(std::move(f), std::move(cb));
  if (res) {
    while (!propose_end_) {
      cond_.Wait();
    }
  }
  return res;
}

void Group::ProposeComplete(void* context, const Status& result,
                            uint64_t instance_id) {
  MutexLock lock(&mutex_);
  propose_end_ = true;
  result_ = result;
  cond_.Signal();
  LOG_DEBUG("Group %u - %s", config_.GetGroupId(), result.ToString().c_str());
}

void Group::GetMembership(std::vector<Member>* result,
                          uint64_t* version) const {
  result->clear();
  std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
  *version = temp->version();
  Member m;
  for (auto& i : temp->members()) {
    m.id = i.second.id();
    m.host = i.second.host();
    m.port = static_cast<uint16_t>(i.second.port());
    m.context = i.second.context();
    result->push_back(m);
  }
}

bool Group::GetMaster(Member* i, uint64_t* version) const {
  uint64_t node_id;
  if (master_machine_->GetMaster(&node_id, version)) {
    std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
    auto it = temp->members().find(node_id);
    if (it != temp->members().end()) {
      i->id = it->second.id();
      i->host = it->second.host();
      i->port = static_cast<uint16_t>(it->second.port());
      i->context = it->second.context();
      return true;
    }
  }
  return false;
}

bool Group::IsMaster() const { return master_machine_->IsMaster(); }

void Group::RetireMaster() {
  if (schedule_->MasterLoop()) {
    schedule_->MasterLoop()->QueueInLoop([this]() { retrie_master_ = true; });
  } else {
    LOG_WARN("Group %u - You don't use master.", config_.GetGroupId());
  }
}

void Group::StartGC() { config_.GetLogManager()->StartGC(); }

void Group::StopGC() { config_.GetLogManager()->StopGC(); }

}  // namespace skywalker
