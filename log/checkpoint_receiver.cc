// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_receiver.h"

#include <vector>

#include "log/checkpoint_manager.h"
#include "paxos/config.h"
#include "skywalker/checkpoint.h"
#include "skywalker/file.h"
#include "skywalker/logging.h"
#include "util/timeops.h"

namespace skywalker {

CheckpointReceiver::CheckpointReceiver(Config* config,
                                       CheckpointManager* manager)
    : config_(config), manager_(manager) {}

CheckpointReceiver::~CheckpointReceiver() {}

bool CheckpointReceiver::BeginToReceive(const CheckpointMessage& msg) {
  sender_node_id_ = msg.node_id();
  sequence_id_ = 0;
  dirs_.clear();

  bool res = true;
  std::vector<std::string> dirs;
  std::vector<std::string> files;
  FileManager::Instance()->GetChildren(config_->CheckpointPath(), &dirs);

  for (auto& dir : dirs) {
    std::string d = config_->CheckpointPath() + "/" + dir;
    FileManager::Instance()->GetChildren(d, &files, true);
    if (files.empty()) {
      continue;
    }
    for (auto& file : files) {
      Status del = FileManager::Instance()->DeleteFile(d + "/" + file);
      if (!del.ok()) {
        LOG_ERROR("Group %u - %s.", config_->GetGroupId(),
                  del.ToString().c_str());
        res = false;
        break;
      }
    }
    FileManager::Instance()->DeleteDir(d);
  }

  return ComfirmReceive(msg, res);
}

bool CheckpointReceiver::ReceiveCheckpoint(const CheckpointMessage& msg) {
  LOG_DEBUG("Group %u - sequence_id(%d==%d)", config_->GetGroupId(),
            msg.sequence_id(), sequence_id_ + 1);
  bool res = ReceiveFiles(msg);
  return ComfirmReceive(msg, res);
}

bool CheckpointReceiver::ReceiveFiles(const CheckpointMessage& msg) {
  if (msg.node_id() != sender_node_id_) {
    return false;
  }

  if (msg.sequence_id() == sequence_id_) {
    return true;
  }

  if (msg.sequence_id() != sequence_id_ + 1) {
    return false;
  }

  if (dirs_.find(msg.machine_id()) == dirs_.end()) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/m%d", config_->CheckpointPath().c_str(),
             msg.machine_id());
    FileManager::Instance()->CreateDir(dir);
    bool exits = FileManager::Instance()->FileExists(dir);
    if (!exits) {
      LOG_ERROR("Group %u - create checkpoint dir=%s failed.",
                config_->GetGroupId(), dir);
      return false;
    }
    dirs_[msg.machine_id()] = std::string(dir);
  }

  WritableFile* file;
  std::string fname = dirs_[msg.machine_id()] + "/" + msg.file();
  Status s = FileManager::Instance()->NewAppendableFile(fname, &file);
  if (s.ok()) {
    s = file->Append(msg.data());
    if (s.ok()) {
      ++sequence_id_;
    }
    delete file;
  }
  if (!s.ok()) {
    LOG_ERROR("Group %u - %s.", config_->GetGroupId(), s.ToString().c_str());
    return false;
  }
  return true;
}

bool CheckpointReceiver::EndToReceive(const CheckpointMessage& msg) {
  if (msg.node_id() != sender_node_id_) {
    return true;
  }

  if (msg.sequence_id() != sequence_id_ + 1) {
    return false;
  }

  bool res = true;

  bool lock = false;
  while (!lock) {
    lock = config_->GetCheckpoint()->LockCheckpoint(config_->GetGroupId());
    SleepForMicroseconds(1000);
  }
  std::vector<std::string> files;
  for (auto& d : dirs_) {
    Status s = FileManager::Instance()->GetChildren(d.second, &files, true);
    if (!s.ok()) {
      LOG_ERROR("Group %u - %s", config_->GetGroupId(), s.ToString().c_str());
      res = false;
      break;
    }
    if (files.empty()) {
      continue;
    }
    res = config_->GetCheckpoint()->LoadCheckpoint(
        config_->GetGroupId(), msg.instance_id(), d.first, d.second, files);
    if (!res) {
      LOG_ERROR("Group %u - load checkpoint failed, the machine_id=%d, dir=%s.",
                config_->GetGroupId(), d.first, d.second.c_str());
      break;
    }
  }
  config_->GetCheckpoint()->UnLockCheckpoint(config_->GetGroupId());

  if (res) {
    LOG_INFO("Group %u - load checkpoint successful, so exit process now!",
             config_->GetGroupId());
    exit(-1);
  }
  return res;
}

bool CheckpointReceiver::ComfirmReceive(const CheckpointMessage& msg,
                                        bool res) {
  CheckpointMessage* reply_msg = new CheckpointMessage();
  reply_msg->set_type(CHECKPOINT_COMFIRM);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_sequence_id(msg.sequence_id());
  reply_msg->set_flag(res);
  config_->GetMessager()->SendMessage(
      msg.node_id(), config_->GetMessager()->PackMessage(reply_msg));
  if (!res && msg.node_id() == sender_node_id_) {
    Reset();
    return false;
  }
  return true;
}

void CheckpointReceiver::Reset() {
  sender_node_id_ = 0;
  sequence_id_ = 0;
  dirs_.clear();
}

}  // namespace skywalker
