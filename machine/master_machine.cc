// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/master_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"
#include "util/timeops.h"
#include "skywalker/file.h"

namespace skywalker {

MasterMachine::MasterMachine(Config* config)
    : config_(config), has_call_(false) {
  set_machine_id(2);
}

bool MasterMachine::Recover(uint32_t group_id, uint64_t instance_id,
                            const std::string& dir) {
  std::string data;
  if (!StateMachine::ReadCheckpoint(
      group_id, instance_id, dir + "/" + kMasterCheckpoint, &state_)) {
    return true;
  }
  if (state_.node_id() != config_->GetNodeId()) {
    state_.set_lease_time(NowMillis() + state_.lease_time());
  } else {
    state_.set_lease_time(NowMillis());
  }
  LOG_INFO("Group %u - instance %llu master recover success.",
            group_id, (unsigned long long)instance_id);
  return true;
}

bool MasterMachine::MakeCheckpoint(uint32_t group_id,
                                   uint64_t instance_id,
                                   const std::string& dir,
                                   const FinishCheckpointCallback& cb) {
  config_->GetCleanLoop()->QueueInLoop(
      [this, group_id, instance_id, dir, cb, state = state_]() {
    bool b = StateMachine::WriteCheckpoint(
        group_id, instance_id, dir + "/" + kMasterCheckpoint, state);
    if (b) {
      LOG_INFO("Group %u - instance %llu master make checkpoint success.",
                group_id, (unsigned long long)instance_id);
    }
    cb(machine_id(), group_id, instance_id, b);
  });
  return true;
}

bool MasterMachine::Execute(uint32_t group_id, uint64_t instance_id,
                            const std::string& value, void* context) {
  if (instance_id <= state_.version()) {
    LOG_ERROR("Group %u - instance(id=%llu) < state version(%llu)",
              group_id, (unsigned long long)instance_id,
              (unsigned long long)state_.version());
    return false;
  }
  MasterState state;
  if (state.ParseFromString(value)) {
    state.set_version(instance_id);

    uint64_t* now = reinterpret_cast<uint64_t*>(context);
    if (now) {
      state.set_lease_time(*now + state.lease_time());
    } else {
      state.set_lease_time(NowMillis() + state.lease_time());
    }
    SetMasterState(state);
    LOG_INFO(
        "Group %u - now the master's version=%llu, "
        "node_id=%llu, lease_time=%llu.",
        config_->GetGroupId(), (unsigned long long)state.version(),
        (unsigned long long)state.node_id(),
        (unsigned long long)state.lease_time());
    if (cb_) {
      cb_(group_id);
    }
    return true;
  } else {
    LOG_ERROR("Group %u - master state parse from string failed.",
              config_->GetGroupId());
  }
  return false;
}

bool MasterMachine::GetCheckpoint(uint32_t group_id, uint64_t instance_id,
                                  const std::string& dir,
                                  std::vector<std::string>* files) {
  files->push_back(kMasterCheckpoint);
  return true;
}

void MasterMachine::SetMasterState(const MasterState& state) {
  std::lock_guard<std::mutex> lock(mutex_);
  state_ = state;
}

MasterState MasterMachine::GetMasterState() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return state_;
}

bool MasterMachine::GetMaster(uint64_t* node_id, uint64_t* version) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (state_.lease_time() > NowMicros()) {
    *version = state_.version();
    *node_id = state_.node_id();
    return true;
  }
  return false;
}

bool MasterMachine::IsMaster() const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (state_.node_id() == config_->GetNodeId() &&
      state_.lease_time() > NowMillis()) {
    return true;
  }
  return false;
}

}  // namespace skywalker
