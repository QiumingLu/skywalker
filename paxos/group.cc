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
      MachineContext* context(
          new MachineContext(membership_machine_->machine_id()));
      bool res = NewPropose(
          std::bind(&Group::SyncMembershipInLoop, this, context));
      if (res) {
        if (result_.ok() || result_.IsConflict()) {
          break;
        }
      } else {
        delete context;
      }
    } else {
      break;
    }
    SleepForMicroseconds(500 * 1000);
  }
}

void Group::SyncMembershipInLoop(MachineContext* context) {
  if (!membership_machine_->HasSyncMembership()) {
    std::string s;
    std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
    MemberChangeMessage message;
    for (auto& i : temp->members()) {
      *(message.add_member()) = i.second;
      message.add_type(MEMBER_ADD);
    }
    message.SerializeToString(&s);
    instance_.OnPropose(s, context);
  } else {
    propose_cb_(context, Status::OK(), instance_.GetInstanceId());
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
    MachineContext* context(
        new MachineContext(master_machine_->machine_id(),
                           reinterpret_cast<void*>(&lease_time)));
    ProposeHandler f(std::bind(&Group::TryBeMasterInLoop, this, context));
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
    } else {
      delete context;
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

void Group::TryBeMasterInLoop(MachineContext* context) {
  MasterState state(master_machine_->GetMasterState());
  if (state.lease_time() <= NowMicros() || state.node_id() == node_id_) {
    state.set_node_id(node_id_);
    state.set_lease_time(lease_timeout_);
    std::string s;
    state.SerializeToString(&s);
    instance_.OnPropose(s, context);
  } else {
    propose_cb_(context, Status::Conflict(Slice()),
                instance_.GetInstanceId());
  }
}

bool Group::OnPropose(const std::string& value,
                      MachineContext* context,
                      const ProposeCompleteCallback& cb) {
  return propose_queue_.Put(
      std::bind(&Instance::OnPropose, &instance_, value, context), cb);
}

bool Group::OnPropose(const std::string& value,
                      MachineContext* context,
                      ProposeCompleteCallback&& cb) {
  return propose_queue_.Put(
      std::bind(&Instance::OnPropose, &instance_, value, context),
      std::move(cb));
}

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  schedule_->IOLoop()->QueueInLoop([c, this]() {
    instance_.OnReceiveContent(c);
  });
}

bool Group::AddMember(const Member& i,
                      const MembershipCompleteCallback& cb) {
  MachineContext* context(
      new MachineContext(membership_machine_->machine_id()));
  bool res = propose_queue_.Put(
      std::bind(&Group::AddMemberInLoop, this, i, context),
      [cb](MachineContext* c, const Status& s, uint64_t instance_id) {
    cb(s, instance_id);
    delete c;
  });
  if (!res) {
    delete context;
  }
  return res;
}

void Group::AddMemberInLoop(const Member& i, MachineContext* context) {
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
    instance_.OnPropose(s, context);
  } else {
    propose_cb_(context, Status::OK(), instance_.GetInstanceId());
  }
}

bool Group::RemoveMember(const Member& i,
                         const MembershipCompleteCallback& cb) {
  MachineContext* context(
      new MachineContext(membership_machine_->machine_id()));
  bool res = propose_queue_.Put(
      std::bind(&Group::RemoveMemberInLoop, this, i, context),
      [cb](MachineContext* c, const Status& s, uint64_t instance_id) {
    cb(s, instance_id);
    delete c;
  });
  if (res) {
    delete context;
  }
  return res;
}

void Group::RemoveMemberInLoop(const Member& i, MachineContext* context) {
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
    instance_.OnPropose(s, context);
  } else {
    propose_cb_(context, Status::OK(), instance_.GetInstanceId());
  }
}

bool Group::ReplaceMember(const Member& i, const Member& j,
                          const MembershipCompleteCallback& cb) {
  MachineContext* context(
      new MachineContext(membership_machine_->machine_id()));
  bool res = propose_queue_.Put(
      std::bind(&Group::ReplaceMemberInLoop, this, i, j, context),
      [cb](MachineContext* c, const Status& s, uint64_t instance_id) {
    cb(s, instance_id);
    delete c;
  });
  if (!res) {
    delete context;
  }
  return res;
}

void Group::ReplaceMemberInLoop(const Member& i, const Member& j,
                                MachineContext* context) {
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
    instance_.OnPropose(s, context);
  } else {
    propose_cb_(context, Status::OK(), instance_.GetInstanceId());
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

void Group::ProposeComplete(MachineContext* context,
                            const Status& result,
                            uint64_t instance_id) {
  MutexLock lock(&mutex_);
  delete context;
  result_ = result;
  propose_end_ = true;
  cond_.Signal();
  LOG_DEBUG("Group %u - %s", config_.GetGroupId(), result_.ToString().c_str());
}

void Group::GetMembership(std::vector<Member>* result, uint64_t* version) const {
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

}  // namespace skywalker
