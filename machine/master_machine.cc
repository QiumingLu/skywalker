// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/master_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"
#include "util/timeops.h"

namespace skywalker {

MasterMachine::MasterMachine(Config* config)
    : config_(config), has_call_(false) {
  set_machine_id(1);
}

void MasterMachine::Recover() {
  int ret = config_->GetDB()->GetMasterState(&state_);
  if (ret == 0) {
    if (state_.node_id() != config_->GetNodeId()) {
      state_.set_lease_time(NowMicros() + state_.lease_time());
    } else {
      state_.set_lease_time(NowMicros());
    }
  }
}

bool MasterMachine::Execute(uint32_t group_id, uint64_t instance_id,
                            const std::string& value, void* context) {
  MasterState state;
  if (state.ParseFromString(value)) {
    if (instance_id < state_.version()) {
      return true;
    }
    state.set_version(instance_id);
    int ret = config_->GetDB()->SetMasterState(state);
    if (ret == 0) {
      bool b = (state.node_id() != state_.node_id());
      uint64_t* now = reinterpret_cast<uint64_t*>(context);
      if (now) {
        state.set_lease_time(*now + state.lease_time());
      } else {
        state.set_lease_time(NowMicros() + state.lease_time());
      }
      SetMasterState(state);
      LOG_INFO(
          "Group %u - now the master's version=%llu, "
          "node_id=%llu, lease_time=%llu.",
          config_->GetGroupId(), (unsigned long long)state.version(),
          (unsigned long long)state.node_id(),
          (unsigned long long)state.lease_time());
      if ((!has_call_ || b) && cb_) {
        has_call_ = true;
        cb_(group_id);
      }
      return true;
    } else {
      LOG_ERROR("Group %u - update master state failed.",
                config_->GetGroupId());
    }
  } else {
    LOG_ERROR("Group %u - master state parse from string failed.",
              config_->GetGroupId());
  }
  return false;
}

std::string MasterMachine::GetString() const {
  std::string s;
  if (state_.lease_time() > NowMicros()) {
    state_.SerializeToString(&s);
  }
  return s;
}

void MasterMachine::SetString(const std::string& s) {
  MasterState state;
  state.ParseFromString(s);
  if (state.version() > state_.version()) {
    SetMasterState(state);
  }
}

void MasterMachine::SetMasterState(const MasterState& state) {
  std::unique_lock<std::mutex> lock(mutex_);
  state_ = state;
}

MasterState MasterMachine::GetMasterState() const {
  std::unique_lock<std::mutex> lock(mutex_);
  return state_;
}

bool MasterMachine::GetMaster(uint64_t* node_id, uint64_t* version) const {
  std::unique_lock<std::mutex> lock(mutex_);
  if (state_.lease_time() > NowMicros()) {
    *version = state_.version();
    *node_id = state_.node_id();
    return true;
  }
  return false;
}

bool MasterMachine::IsMaster() const {
  std::unique_lock<std::mutex> lock(mutex_);
  if (state_.node_id() == config_->GetNodeId() &&
      state_.lease_time() > NowMicros()) {
    return true;
  }
  return false;
}

}  // namespace skywalker
