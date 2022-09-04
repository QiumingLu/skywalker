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
                            const std::string& dir,
                            const std::vector<std::string>& files) {
  if (dir.empty() || files.empty()) {
    return true;
  }
  std::string data;
  Status status = ReadFileToString(FileManager::Instance(), files[0], &data);
  if (!status.ok()) {
    LOG_ERROR("Group %u - instance %llu master read failed %s.",
              config_->GetGroupId(), instance_id, status.ToString().c_str());
    return true;
  }
  if (state_.ParseFromString(data)) {
    if (state_.node_id() != config_->GetNodeId()) {
      state_.set_lease_time(NowMicros() + state_.lease_time());
    } else {
      state_.set_lease_time(NowMicros());
    }
  } else {
    LOG_ERROR("Group %u - instance %llu master parse failed.",
              config_->GetGroupId(), instance_id);   
  }
  return true;
}

bool MasterMachine::Execute(uint32_t group_id, uint64_t instance_id,
                            const std::string& value, void* context) {
  MasterState state;
  if (state.ParseFromString(value)) {
    if (instance_id <= state_.version()) {
      return false;
    }
    state.set_version(instance_id);

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

bool MasterMachine::MakeCheckpoint(uint32_t group_id,
                                   uint64_t instance_id,
                                   const std::string& dir) {
  if (state_.node_id() == 0) {
    return true;
  }
  Status status = WriteStringToFileSync(
      FileManager::Instance(), state_.SerializeAsString(), dir + "/masterdb");
  if (!status.ok()) {
    LOG_ERROR("Group %u - instance %llu master write failed %s.",
              config_->GetGroupId(), instance_id, status.ToString().c_str());
    return false;
  }
  return true;
}

}  // namespace skywalker
