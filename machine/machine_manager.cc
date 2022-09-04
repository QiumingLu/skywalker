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

std::string GetCheckpointPath(Config* config, uint64_t instance_id, bool temp) {
  if (temp) {
    return config->TempCheckpointPath();
  } else {
    return config->CheckpointPath() + "/Checkpoint-" + std::to_string(instance_id);
  }
}

std::string GetCheckpointPath(
    Config* config, uint64_t instance_id, uint32_t machine_id, bool temp) {
  if (temp) {
    return config->TempCheckpointPath() + "/m" + std::to_string(machine_id); 
  } else {
    return config->CheckpointPath() +
           "/Checkpoint-" + std::to_string(instance_id) +
           "/m" + std::to_string(machine_id); 
  }
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
  return res;
}

MachineManager::MachineManager(Config* config)
    : config_(config),
      generator_((unsigned)NowMillis()),
      distribution_(1, config_->KeepLogCount() / 2),
      latest_checkpoint_instance_id_(0) {}

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

  for (const auto& it : machines_) {
    std::string dir;
    std::vector<std::string> files;
    if (!GetCheckpoint(latest_checkpoint_instance_id_, it.first, &dir, &files)) {
      LOG_ERROR("Group %u - machine(id=%u) get checkpoint failed.",
                config_->GetGroupId(), it.first);
      return false;
    }
    if (it.second->Recover(config_->GetGroupId(),
                           latest_checkpoint_instance_id_, dir, files)) {
      LOG_INFO("Group %u - machine(id=%u) instance(id=%llu) recover success.",
              config_->GetGroupId(), it.first, latest_checkpoint_instance_id_);
    } else {
      LOG_ERROR("Group %u - machine(id=%u) instance(id=%llu) recover failed.",
                config_->GetGroupId(), it.first, latest_checkpoint_instance_id_);
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
             config_->GetGroupId(), value.machine_id(), instance_id, log.c_str());
  } else {
    LOG_ERROR("Group %u - machine(id=%u) is not existed.",
              config_->GetGroupId(), value.machine_id());
  }
  if (result) {
    MakeCheckpoint(instance_id);
  }
  return false;
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
  assert(instance_id > latest_checkpoint_instance_id_);
  uint64_t interval = instance_id - latest_checkpoint_instance_id_;
  if (interval < (config_->KeepLogCount() / 2 +  distribution_(generator_))) {
    return false;
  }
  if (!DeleteTempCheckpoint(config_)) {
    return false;
  }
  bool result = true;
  for (const auto& it : machines_) {
    std::string dir = GetCheckpointPath(config_, instance_id, it.first, true);
    Status status = FileManager::Instance()->CreateDir(dir);
    if (!status.ok()) {
      LOG_ERROR("Group %u - instance(id=%llu) create dir %s.",
                config_->GetGroupId(), instance_id, status.ToString().c_str());
      return false;
    }
    if (it.second->MakeCheckpoint(config_->GetGroupId(), instance_id, dir)) {
      LOG_INFO("Group %u - machine(id=%u) instance(id=%llu) make checkpoint success.",
               config_->GetGroupId(), it.first, instance_id);
    } else {
      LOG_ERROR("Group %u - machine(id=%u) instance(id=%llu) make checkpoint failed.",
                config_->GetGroupId(), it.first, instance_id);
      return false;
    }
  }
  UpdateCheckpoint(instance_id);
  CleanCheckpoint();
  return result;
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
    *dir = GetCheckpointPath(config_, instance_id, machine_id, false);
    if (!FileManager::Instance()->FileExists(*dir)) {
      LOG_WARN("Group %u - machine(id=%u) not has checkpoint.",
               config_->GetGroupId(), machine_id);
      return false;
    }
    Status status = FileManager::Instance()->GetChildren(*dir, files, true);
    if (status.ok()) {
      return true;
    }
    LOG_ERROR("Group %u - machine(id=%u) checkpoint open failed %s.",
              config_->GetGroupId(), machine_id, status.ToString().c_str());
  } else {
    LOG_ERROR("Group %u - machine(id=%u) is not existed.",
              config_->GetGroupId(), machine_id);
  }
  return false;
}

bool MachineManager::UpdateCheckpoint(uint64_t instance_id) {
  Status status = FileManager::Instance()->RenameFile(
      GetCheckpointPath(config_, instance_id, true),
      GetCheckpointPath(config_, instance_id, false));
  latest_checkpoint_instance_id_ = instance_id;
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
  while (checkpoints_.size() > 5) {
    oldest_checkpoint_instance_id_ = checkpoints_[1];
    dirs.clear();
    std::string path = GetCheckpointPath(config_, checkpoints_.front(), false);
    FileManager::Instance()->GetChildren(path, &dirs);
    for (auto& dir : dirs) {
      std::vector<std::string> files;
      FileManager::Instance()->GetChildren(dir, &files, true);
      for (auto& file : files) {
        FileManager::Instance()->DeleteFile(file);
      }
      FileManager::Instance()->DeleteDir(dir);    
    }
    Status status = FileManager::Instance()->DeleteDir(path);
    if (!status.ok()) {
        LOG_ERROR("Group %u - checkpoint delete failed %s.",
                  config_->GetGroupId(), status.ToString().c_str());
        FileManager::Instance()->RenameFile(
          path, config_->CheckpointPath() + "/deletedcheckpoint");
    }
    std::swap(checkpoints_.front(), checkpoints_.back());
    checkpoints_.pop_back();
  }
  UnLockCheckpoint();
}

}  // namespace skywalker
