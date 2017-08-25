// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/log_manager.h"

#include <string>

#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

LogManager::LogManager(Config* config)
    : config_(config),
      min_chosen_id_(0),
      max_chosen_id_(0),
      cleaner_(config, this) {}

bool LogManager::Recover(uint64_t* instance_id) {
  uint64_t temp = 0;
  if (config_->GetDB()->GetMinChosenInstanceId(&temp) == -1) {
    return false;
  }
  uint64_t id = config_->GetCheckpointManager()->GetCheckpointInstanceId() + 1;

  if (id > *instance_id) {
    *instance_id = id;
  }

  min_chosen_id_ = temp;
  max_chosen_id_ = *instance_id;

  if (id < *instance_id) {
    return ReplayLog(id, *instance_id);
  }
  return true;
}

bool LogManager::ReplayLog(uint64_t from, uint64_t to) {
  for (uint64_t instance_id = from; instance_id < to; ++instance_id) {
    std::string s;
    int res = config_->GetDB()->Get(instance_id, &s);
    if (res != 0) {
      LOG_ERROR("Group %u - replay log failed, the instance_id=%llu.",
                config_->GetGroupId(), (unsigned long long)instance_id);
      return false;
    }
    PaxosInstance temp;
    temp.ParseFromString(s);
    const PaxosValue& value = temp.accepted_value();
    config_->GetMachineManager()->Execute(instance_id, value, nullptr);
  }
  LOG_INFO("Group %u - replay log successful, from %llu to %llu.",
           config_->GetGroupId(), (unsigned long long)from,
           (unsigned long long)to);
  return true;
}

void LogManager::StartGC() { cleaner_.StartGC(); }

void LogManager::StopGC() { cleaner_.StopGC(); }

uint64_t LogManager::GetMinChosenInstanceId() const { return min_chosen_id_; }

void LogManager::SetMinChosenInstanceId(uint64_t id) {
  int res = config_->GetDB()->SetMinChosenInstanceId(id);
  if (res == 0) {
    min_chosen_id_ = id;
  }
}

uint64_t LogManager::GetMaxChosenInstanceId() const { return max_chosen_id_; }

void LogManager::SetMaxChosenInstanceId(uint64_t id) { max_chosen_id_ = id; }

}  // namespace skywalker
