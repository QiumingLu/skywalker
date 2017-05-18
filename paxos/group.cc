// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/group.h"

#include <utility>

#include "util/mutexlock.h"
#include "util/timeops.h"
#include "skywalker/logging.h"

namespace skywalker {

Group::Group(uint64_t node_id,
             const GroupOptions& options, Network* network)
    : node_id_(node_id),
      config_(node_id, options, network),
      instance_(&config_),
      lease_timeout_(10 * 1000 * 1000),
      retrie_master_(false),
      mutex_(),
      cond_(&mutex_),
      propose_end_(false),
      propose_queue_(100),
      schedule_(new Schedule(options.use_master)) {
  propose_cb_ =  std::bind(&ProposeQueue::ProposeComplete,
                           &propose_queue_,
                           std::placeholders::_1,
                           std::placeholders::_2,
                           std::placeholders::_3);
  instance_.SetProposeCompleteCallback(propose_cb_);

  membership_machine_ =  config_.GetMembershipMachine();
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
    if (++i > 3) {
      instance_.SyncData(false);
    }
    if (!membership_machine_->HasSyncMembership()) {
      bool res = NewPropose(std::bind(&Group::SyncMembershipInLoop, this));
      if (res) {
        if (result_.ok() || result_.IsConflict()) {
          break;
        }
      }
    } else {
      break;
    }
    SleepForMicroseconds(500 * 1000);
  }
}

void Group::SyncMembershipInLoop() {
  if (!membership_machine_->HasSyncMembership()) {
    std::string s;
    std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
    MemberChangeMessage message;
    for (auto& i : temp->members()) {
      *(message.add_member()) = i.second;
      message.add_type(MEMBER_ADD);
    }
    message.SerializeToString(&s);
    instance_.OnPropose(s, membership_machine_->machine_id());
  } else {
    propose_cb_(nullptr, Status::OK(), instance_.GetInstanceId());
  }
}

void Group::SyncMaster() {
  if (schedule_->MasterLoop() == nullptr) {
    return;
  }
  schedule_->MasterLoop()->QueueInLoop([this]() {
    TryBeMaster();
  });
}

void Group::TryBeMaster() {
  MasterState state(master_machine_->GetMasterState());
  uint64_t next_time = state.lease_time();
  uint64_t now = NowMicros();
  if (state.lease_time() <= now ||
      (state.node_id() == node_id_ && !retrie_master_)) {
    uint64_t lease_time = now + lease_timeout_;
    ProposeHandler f(std::bind(&Group::TryBeMasterInLoop, this, &lease_time));
    bool res = NewPropose(std::move(f));
    next_time = 0;
    if (res) {
      state = master_machine_->GetMasterState();
      if (state.node_id() == node_id_) {
        if (result_.ok()) {
          next_time = state.lease_time() - 30 * 1000;
        } else if (result_.IsConflict()) {
          next_time = state.lease_time();
        }
      }
    }
    if (next_time == 0) {
      next_time = NowMicros() + lease_timeout_;
    }
  }
  if (retrie_master_) {
    retrie_master_ = false;
  }

  schedule_->MasterLoop()->RunAt(next_time, [this]() {
    TryBeMaster();
  });
}

void Group::TryBeMasterInLoop(void* context) {
  MasterState state(master_machine_->GetMasterState());
  if (state.lease_time() <= NowMicros() || state.node_id() == node_id_) {
    state.set_node_id(node_id_);
    state.set_lease_time(lease_timeout_);
    std::string s;
    state.SerializeToString(&s);
    instance_.OnPropose(s, master_machine_->machine_id(), context);
  } else {
    propose_cb_(context, Status::Conflict(Slice()),
                instance_.GetInstanceId());
  }
}

bool Group::OnPropose(const std::string& value,
                      int machine_id, void* context,
                      const ProposeCompleteCallback& cb) {
  return propose_queue_.Put(
      std::bind(&Instance::OnPropose, &instance_, value, machine_id, context),
      cb);
}

bool Group::OnPropose(const std::string& value,
                      int machine_id, void* context,
                      ProposeCompleteCallback&& cb) {
  return propose_queue_.Put(
      std::bind(&Instance::OnPropose, &instance_, value, machine_id, context),
      std::move(cb));
}

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  schedule_->IOLoop()->QueueInLoop([c, this]() {
    instance_.OnReceiveContent(c);
  });
}

bool Group::AddMember(const Member& i,
                      const MembershipCompleteCallback& cb) {
  return propose_queue_.Put(
      std::bind(&Group::AddMemberInLoop, this, i),
      [cb](void* context, const Status& s, uint64_t id) { cb(s, id); });
}

void Group::AddMemberInLoop(const Member& i) {
  std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
  if (temp->members().find(i.id) == temp->members().end()) {
    MemberMessage msg;
    msg.set_id(i.id);
    msg.set_ip(i.ip);
    msg.set_port(i.port);
    msg.set_context(i.context);
    MemberChangeMessage change;
    *(change.add_member()) = msg;
    change.add_type(MEMBER_ADD);
    std::string s;
    change.SerializeToString(&s);
    instance_.OnPropose(s, membership_machine_->machine_id());
  } else {
    propose_cb_(nullptr, Status::OK(), instance_.GetInstanceId());
  }
}

bool Group::RemoveMember(const Member& i,
                         const MembershipCompleteCallback& cb) {
  return propose_queue_.Put(
      std::bind(&Group::RemoveMemberInLoop, this, i),
      [cb](void* context, const Status& s, uint64_t id) { cb(s, id); });
}

void Group::RemoveMemberInLoop(const Member& i) {
  std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
  if (temp->members().find(i.id) != temp->members().end()) {
    MemberMessage msg;
    msg.set_id(i.id);
    msg.set_ip(i.ip);
    msg.set_port(i.port);
    msg.set_context(i.context);
    MemberChangeMessage change;
    *(change.add_member()) = msg;
    change.add_type(MEMBER_REMOVE);
    std::string s;
    change.SerializeToString(&s);
    instance_.OnPropose(s, membership_machine_->machine_id());
  } else {
    propose_cb_(nullptr, Status::OK(), instance_.GetInstanceId());
  }
}

bool Group::ReplaceMember(const Member& i, const Member& j,
                          const MembershipCompleteCallback& cb) {
  return propose_queue_.Put(
      std::bind(&Group::ReplaceMemberInLoop, this, i, j),
      [cb](void* context, const Status& s, uint64_t id) { cb(s, id); });
}

void Group::ReplaceMemberInLoop(const Member& i, const Member& j) {
  std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
  MemberChangeMessage change;
  if (temp->members().find(i.id) == temp->members().end()) {
    MemberMessage msg;
    msg.set_id(i.id);
    msg.set_ip(i.ip);
    msg.set_port(i.port);
    msg.set_context(i.context);
    *(change.add_member()) = msg;
    change.add_type(MEMBER_ADD);
  }
  if (temp->members().find(j.id) != temp->members().end()) {
    MemberMessage msg;
    msg.set_id(j.id);
    msg.set_ip(j.ip);
    msg.set_port(j.port);
    msg.set_context(j.context);
    *(change.add_member()) = msg;
    change.add_type(MEMBER_REMOVE);
  }
  if (change.member_size() > 0) {
    std::string s;
    change.SerializeToString(&s);
    instance_.OnPropose(s, membership_machine_->machine_id());
  } else {
    propose_cb_(nullptr, Status::OK(), instance_.GetInstanceId());
  }
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

void Group::ProposeComplete(void* context,
                            const Status& result,
                            uint64_t instance_id) {
  MutexLock lock(&mutex_);
  result_ = result;
  propose_end_ = true;
  cond_.Signal();
  LOG_DEBUG("Group %u - %s", config_.GetGroupId(), result_.ToString().c_str());
}

void Group::GetMembership(std::vector<Member>* result,
                          uint64_t* version) const {
  result->clear();
  std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
  *version = temp->version();
  Member m;
  for (auto& i : temp->members()) {
    m.id = i.second.id();
    m.ip = i.second.ip();
    m.port = static_cast<uint16_t>(i.second.port());
    m.context = i.second.context();
    result->push_back(m);
  }
}

void Group::SetMasterLeaseTime(uint64_t micros) {
  if (schedule_->MasterLoop()) {
    schedule_->MasterLoop()->QueueInLoop([micros, this]() {
      if (micros < (5 *1000 * 1000)) {
        lease_timeout_ = 5 * 1000 * 1000;
      } else {
        lease_timeout_ = micros;
      }
    });
  } else {
    LOG_WARN("Group %u - You don't use master.", config_.GetGroupId());
  }
}

bool Group::GetMaster(Member* i, uint64_t* version) const {
  uint64_t node_id;
  if (master_machine_->GetMaster(&node_id, version)) {
    std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
    auto it = temp->members().find(node_id);
    if (it != temp->members().end()) {
      i->id = it->second.id();
      i->ip = it->second.ip();
      i->port = static_cast<uint16_t>(it->second.port());
      i->context = it->second.context();
      return true;
    }
  }
  return false;
}

bool Group::IsMaster() const {
  return master_machine_->IsMaster();
}

void Group::RetireMaster() {
  if (schedule_->MasterLoop()) {
    schedule_->MasterLoop()->QueueInLoop([this]() {
      retrie_master_ = true;
    });
  } else {
    LOG_WARN("Group %u - You don't use master.", config_.GetGroupId());
  }
}

void Group::StartGC() {
  config_.GetLogManager()->StartGC();
}

void Group::StopGC() {
  config_.GetLogManager()->StopGC();
}

}  // namespace skywalker
