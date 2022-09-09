// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/machine_manager.h"

#include <assert.h>
#include <utility>

#include "paxos/config.h"
#include "skywalker/logging.h"
#include "skywalker/file.h"
#include "util/timeops.h"

namespace skywalker {

std::string GetCheckpointPath(Config* config, uint64_t instance_id) {
  return config->CheckpointPath() + "/checkpoint-" + std::to_string(instance_id);
}

std::string GetCheckpointPath(
    Config* config, uint64_t instance_id, uint32_t machine_id) {
  return config->CheckpointPath() +
         "/checkpoint-" + std::to_string(instance_id) +
         "/m" + std::to_string(machine_id);
}

std::string GetTempCheckpointPath(Config* config, uint64_t instance_id) {
  return config->TempCheckpointPath();
}

std::string GetTempCheckpointPath(
    Config* config, uint64_t instance_id, uint32_t machine_id) {
  return config->TempCheckpointPath() + "/m" + std::to_string(machine_id);
}

bool DeleteTempCheckpoint(Config* config) {
  bool res = true;
  std::vector<std::string> dirs;
  std::vector<std::string> files;
  FileManager::Instance()->GetChildren(config->TempCheckpointPath(), &dirs);

  for (auto& dir : dirs) {
    std::string d = config->TempCheckpointPath() + "/" + dir;
    FileManager::Instance()->GetChildren(d, &files, true);
    for (auto& file : files) {
      Status del = FileManager::Instance()->DeleteFile(d + "/" + file);
      if (!del.ok()) {
        LOG_ERROR("Group %u - delete file failed, %s.",
                  config->GetGroupId(), del.ToString().c_str());
        res = false;
        break;
      }
    }
    FileManager::Instance()->DeleteDir(d);
  }
  if (!FileManager::Instance()->FileExists(config->TempCheckpointPath())) {
    FileManager::Instance()->CreateDir(config->TempCheckpointPath());
  }
  return res;
}

MachineManager::MachineManager(Config* config)
    : config_(config),
      generator_((unsigned)NowMillis()),
      distribution_(1, config_->KeepLogCount() / 2),
      latest_checkpoint_instance_id_(0),
      make_checkpoint_(false) {}

void MachineManager::AddMachine(StateMachine* machine) {
  assert(machines_.find(machine->machine_id()) == machines_.end());
  machines_.insert(std::make_pair(machine->machine_id(), machine));
}

void MachineManager::RemoveMachine(StateMachine* machine) {
  machines_.erase(machine->machine_id());
}

bool MachineManager::Recover() {
  CleanCheckpoint();
  latest_checkpoint_instance_id_ = checkpoints_.empty() ? 0 : checkpoints_.back();
  if (latest_checkpoint_instance_id_ == 0) {
    return true;
  }

  for (const auto& it : machines_) {
    std::string dir = GetCheckpointPath(config_,
                                        latest_checkpoint_instance_id_,
                                        it.first);
    if (!FileManager::Instance()->FileExists(dir)) {
      LOG_WARN("Group %u - machine(id=%u) instance(id=%llu) no has checkpoint.",
               config_->GetGroupId(), it.first,
               (unsigned long long)latest_checkpoint_instance_id_);
      continue;
    }
    if (it.second->Recover(config_->GetGroupId(),
                           latest_checkpoint_instance_id_, dir)) {
      LOG_INFO("Group %u - machine(id=%u) instance(id=%llu) recover success.",
              config_->GetGroupId(), it.first,
              (unsigned long long)latest_checkpoint_instance_id_);
    } else {
      LOG_ERROR("Group %u - machine(id=%u) instance(id=%llu) recover failed.",
                config_->GetGroupId(), it.first,
                (unsigned long long)latest_checkpoint_instance_id_);
      return false;
    }
  }
  return true;
}

bool MachineManager::Execute(uint64_t instance_id, const PaxosValue& value,
                             void* context) {
  bool result = false;
  auto it = machines_.find(value.machine_id());
  if (it != machines_.end()) {
    assert(it->second != nullptr);
    result = it->second->Execute(config_->GetGroupId(), instance_id,
                                 value.user_data(), context);
    std::string log = result ? "success" : "failed";
    LOG_INFO("Group %u - machine(id=%u) instance(id=%llu) execute %s.",
             config_->GetGroupId(), value.machine_id(),
             (unsigned long long)instance_id, log.c_str());
  } else {
    LOG_ERROR("Group %u - machine(id=%u) is not existed.",
              config_->GetGroupId(), value.machine_id());
  }
  if (result) {
    MakeCheckpoint(instance_id);
  }
  return result;
}

bool MachineManager::TryLockCheckpoint() {
  return mutex_.try_lock();
}

void MachineManager::LockCheckpoint() {
  mutex_.lock();
}

void MachineManager::UnLockCheckpoint() {
  mutex_.unlock();
}

uint64_t MachineManager::GetLatestCheckpointInstanceId() const {
  return latest_checkpoint_instance_id_;
}

uint64_t MachineManager::GetOldestCheckpointInstanceId() const {
  return oldest_checkpoint_instance_id_;
}

bool MachineManager::MakeCheckpoint(uint64_t instance_id) {
  if (make_checkpoint_) {
    return false;
  }
  if (!config_->GetIOLoop()) {
    return false;
  }

  assert(instance_id > latest_checkpoint_instance_id_);
  uint64_t interval = instance_id - latest_checkpoint_instance_id_;
  if (interval < (config_->KeepLogCount() / 2 +  distribution_(generator_))) {
    return false;
  }
  if (!DeleteTempCheckpoint(config_)) {
    return false;
  }

  make_checkpoint_result_.clear();
  for (const auto& it : machines_) {
    std::string dir = GetTempCheckpointPath(config_, instance_id, it.first);
    Status status = FileManager::Instance()->CreateDir(dir);
    if (!status.ok()) {
      LOG_ERROR("Group %u - instance(id=%llu) create dir %s.",
                config_->GetGroupId(), (unsigned long long)instance_id,
                status.ToString().c_str());
      return false;
    }
    bool b = it.second->MakeCheckpoint(
        config_->GetGroupId(), instance_id, dir,
        [this](uint32_t machine_id, uint32_t group_id, uint64_t id, bool result){
          FinishMakeCheckpoint(machine_id, group_id, id, result);
        });
    if (!b) {
      return false;
    }
    make_checkpoint_result_.insert(it.first);
  }
  if (!make_checkpoint_result_.empty()) {
    make_checkpoint_ = true;
  }
  return true;
}

void MachineManager::FinishMakeCheckpoint(uint32_t machine_id, uint32_t group_id,
                                          uint64_t instance_id, bool result) {
  if (result) {
    LOG_INFO("Group %u - machine(id=%u) instance(id=%llu) make checkpoint success.",
            config_->GetGroupId(), machine_id, (unsigned long long)instance_id);
  } else {
    LOG_ERROR("Group %u - machine(id=%u) instance(id=%llu) make checkpoint failed.",
              config_->GetGroupId(), machine_id, (unsigned long long)instance_id);
  }
  config_->GetIOLoop()->QueueInLoop([this, machine_id, instance_id, result]() {
    if (!make_checkpoint_) {
      return;
    }
    if (!result) {
      make_checkpoint_ = false;
      return;
    }
    make_checkpoint_result_.erase(machine_id);
    if (make_checkpoint_result_.empty()) {
      UpdateCheckpoint(instance_id);
      CleanCheckpoint();
      make_checkpoint_ = false;
    }
  });
}

bool MachineManager::GetCheckpoint(uint64_t instance_id,
                                   uint32_t machine_id,
                                   std::string* dir,
                                   std::vector<std::string>* files) {
  if (instance_id == 0) {
    return true;
  }
  auto it = machines_.find(machine_id);
  if (it != machines_.end()) {
    assert(it->second != nullptr);
    *dir = GetCheckpointPath(config_, instance_id, machine_id);
    if (!FileManager::Instance()->FileExists(*dir)) {
      LOG_WARN("Group %u - machine(id=%u) not has checkpoint.",
               config_->GetGroupId(), machine_id);
      return true;
    }
    if (it->second->GetCheckpoint(config_->GetGroupId(), instance_id, *dir, files)) {
      return true;
    }
  } else {
    LOG_WARN("Group %u - machine(id=%u) is not existed.",
              config_->GetGroupId(), machine_id);
  }
  return false;
}

bool MachineManager::UpdateCheckpoint(uint64_t instance_id) {
  Status status = FileManager::Instance()->RenameFile(
      GetTempCheckpointPath(config_, instance_id),
      GetCheckpointPath(config_, instance_id));
  if (status.ok()) {
    latest_checkpoint_instance_id_ = instance_id;
  } else {
    LOG_ERROR(
        "Group %u - instance(id=%llu) rename faied %s.",
        config_->GetGroupId(), (unsigned long long)instance_id, status.ToString().c_str());
  }
  return status.ok();
}

void MachineManager::CleanCheckpoint() {
  if (!TryLockCheckpoint()) {
    return;
  }
  checkpoints_.clear();
  std::vector<std::string> dirs;
  FileManager::Instance()->GetChildren(config_->CheckpointPath(), &dirs);
  for (auto& dir  : dirs) {
    size_t found = dir.find_first_of("-");
    if (found != std::string::npos) {
      checkpoints_.push_back(std::stoull(dir.substr(found + 1)));
    }
  }
  std::sort(checkpoints_.begin(), checkpoints_.end());
  if (checkpoints_.empty()) {
    oldest_checkpoint_instance_id_ = 0;
  } else {
    oldest_checkpoint_instance_id_ = checkpoints_.front();
  }
  while (checkpoints_.size() > config_->KeepCheckpointCount()) {
    oldest_checkpoint_instance_id_ = checkpoints_[1];
    dirs.clear();
    std::string path = GetCheckpointPath(config_, checkpoints_.front());
    FileManager::Instance()->GetChildren(path, &dirs);
    for (auto& dir : dirs) {
      std::string d = path + "/" + dir;
      std::vector<std::string> files;
      FileManager::Instance()->GetChildren(d, &files, true);
      for (auto& file : files) {
        FileManager::Instance()->DeleteFile(d + "/" + file);
      }
      FileManager::Instance()->DeleteDir(d);
    }
    Status status = FileManager::Instance()->DeleteDir(path);
    if (!status.ok()) {
        LOG_ERROR("Group %u - checkpoint delete failed %s.",
                  config_->GetGroupId(), status.ToString().c_str());
        status = FileManager::Instance()->RenameFile(
            path, config_->CheckpointPath() + "/deletedcheckpoint");
        if (!status.ok()) {
          LOG_ERROR("Group %u - checkpoint rename failed %s.",
                    config_->GetGroupId(), status.ToString().c_str());
        }
    }
    std::swap(checkpoints_.front(), checkpoints_.back());
    checkpoints_.pop_back();
  }
  UnLockCheckpoint();
}

}  // namespace skywalker
