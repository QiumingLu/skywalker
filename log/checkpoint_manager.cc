// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_manager.h"
#include "util/mutexlock.h"
#include "skywalker/logging.h"

namespace skywalker {

CheckpointManager::CheckpointManager(Config* config)
    : config_(config),
      checkpoint_(config_->GetCheckpoint()),
      file_manager_(new FileManager()) {
}

CheckpointManager::~CheckpointManager() {
  delete file_manager_;
}

uint64_t CheckpointManager::GetCheckpointInstanceId() const {
  uint64_t id = -1;
  if (!config_->HasMachines()) {
    int res = config_->GetDB()->GetMaxInstanceId(&id);
    if (res == 0) {
      id = id - 1;
    }
  } else {
    id = checkpoint_->GetCheckpointInstanceId(config_->GetGroupId());
  }
  return id;
}

bool CheckpointManager::SendCheckpoint() {
  bool res = checkpoint_->LockCheckpoint(config_->GetGroupId());
  if (res) {
    std::string dir;
    std::vector<std::string> files;
    checkpoint_->GetCheckpoint(config_->GetGroupId(), &dir, &files);
    if (dir[dir.size() - 1] != '/') {
      dir += '/';
    }
    for (auto& file : files) {
      std::string fname = dir + file;
      res = SendFile(fname);
      if (!res) {
        SWLog(ERROR, "Send file failed, file:%s\n", fname.c_str());
        break;
      }
    }
    checkpoint_->UnLockCheckpoint(config_->GetGroupId());
  }
  return res;
}

bool CheckpointManager::SendFile(const std::string& fname) {
  std::string s;
  Status status = ReadFileToString(file_manager_, fname, &s);
  if (!status.ok()) {
    SWLog(ERROR, "%s\n", status.ToString().c_str());
    return false;
  }
  return true;
}

}  // namespace skywalker
