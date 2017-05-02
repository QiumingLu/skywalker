// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/log_manager.h"
#include "skywalker/logging.h"

namespace skywalker {

LogManager::LogManager(Config* config,
                       CheckpointManager* checkpoint_manager,
                       MachineManager* machine_manager)
    : config_(config),
      checkpoint_manager_(checkpoint_manager),
      machine_manager_(machine_manager),
      min_chosen_id_(0),
      max_chosen_id_(0),
      cleaner_(config, checkpoint_manager_, this) {
}

bool LogManager::Recover(uint64_t instance_id) {
  if (config_->GetDB()->GetMinChosenInstanceId(&min_chosen_id_) == -1) {
    return false;
  }

  bool res = true;
  uint64_t id = checkpoint_manager_->GetCheckpointInstanceId() + 1;
  if (id < instance_id) {
    res = ReplayLog(id, instance_id);
  }
  if (res) {
    cleaner_.Start();
  }
  return res;
}

bool LogManager::ReplayLog(uint64_t from, uint64_t to) {
  for (uint64_t instance_id = from; instance_id < to; ++instance_id) {
    std::string s;
    int res = config_->GetDB()->Get(instance_id, &s);
    if (res != 0) {
      LOG_ERROR("ReplayLog failed, "
                "which group_id:%" PRIu32", instance_id:%" PRIu64".",
                config_->GetGroupId(), instance_id);
      return false;
    }
    AcceptorState state;
    state.ParseFromString(s);
    const PaxosValue& value = state.accepted_value();
    bool success = machine_manager_->Execute(
        value.machine_id(), config_->GetGroupId(), instance_id,
        value.user_data(), nullptr);
    if (!success) {
      LOG_INFO("StateMachine execute failed, which machine_id:%d, "
               "group_id:%" PRIu32", instance_id:%" PRIu64".",
               value.machine_id(), config_->GetGroupId(), instance_id);
    }
  }
  return true;
}

uint64_t LogManager::GetMinChosenInstanceId() const {
  return min_chosen_id_;
}

void LogManager::SetMinChosenInstanceId(uint64_t id) {
  int res = config_->GetDB()->SetMinChosenInstanceId(id);
  if (res == 0) {
    min_chosen_id_ = id;
  }
}

uint64_t LogManager::GetMaxChosenInstanceId() const {
  return max_chosen_id_;
}

void LogManager::SetMaxChosenInstanceId(uint64_t id) {
  max_chosen_id_ = id;
}

}  // namespace skywalker
