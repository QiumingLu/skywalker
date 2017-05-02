// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_receiver.h"
#include "log/checkpoint_manager.h"
#include "skywalker/logging.h"
#include "skywalker/file.h"

namespace skywalker {

CheckpointReceiver::CheckpointReceiver(Config* config,
                                       CheckpointManager* manager)
    : config_(config),
      checkpoint_(config_->GetCheckpoint()),
      messager_(config_->GetMessager()),
      manager_(manager) {
}

CheckpointReceiver::~CheckpointReceiver() {
}

bool CheckpointReceiver::BeginToReceive(const CheckpointMessage& msg) {
  sender_node_id_ = msg.node_id();
  sequence_id_ = 0;
  dirs_.clear();

  bool res = true;
  std::vector<std::string> dirs;
  FileManager::Instance()->GetChildren(config_->CheckpointPath(), &dirs);

  for (auto& dir : dirs) {
    std::string d = config_->CheckpointPath() + "/" + dir;
    std::vector<std::string> files;
    FileManager::Instance()->GetChildren(d, &files, true);
    if (files.empty()) {
      continue;
    }
    for (auto& file : files) {
      Status del = FileManager::Instance()->DeleteFile(d + "/" + file);
      if (!del.ok()) {
        res = false;
      }
    }
    FileManager::Instance()->DeleteDir(d);
  }

  ComfirmReceive(msg, res);
  return res;
}

bool CheckpointReceiver::ReceiveCheckpoint(const CheckpointMessage& msg) {
  bool res = ReceiveFiles(msg);
  ComfirmReceive(msg, res);
  return res;
}

bool CheckpointReceiver::ReceiveFiles(const CheckpointMessage& msg) {
  if (msg.node_id() != sender_node_id_) {
    return false;
  }

  if (msg.sequence_id() ==  sequence_id_) {
    return true;
  }

  if (msg.sequence_id() != sequence_id_ + 1) {
    return false;
  }

  if (dirs_.find(msg.machine_id()) == dirs_.end()) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/machine_%d",
             config_->CheckpointPath().c_str(), msg.machine_id());
    FileManager::Instance()->CreateDir(dir);
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
  return s.ok() ? true : false;
}

bool CheckpointReceiver::EndToReceive(const CheckpointMessage& msg) {
  if (msg.node_id() != sender_node_id_ ||
      msg.sequence_id() != sequence_id_ + 1) {
    ComfirmReceive(msg, false);
    return false;
  }
  const std::vector<StateMachine*>& machines(config_->StateMachines());
  bool res = true;
  for (auto machine : machines) {
    auto it = dirs_.find(machine->machine_id());
    if (it != dirs_.end()) {
      std::vector<std::string> files;
      Status s = FileManager::Instance()->GetChildren(it->second, &files, true);
      if (!s.ok()) {
        LOG_ERROR("%s", s.ToString().c_str());
        res = false;
        break;
      }
      if (files.empty()) {
        continue;
      }
      res = checkpoint_->LoadCheckpoint(config_->GetGroupId(),
                                        it->first, it->second, files);
      if (!res) {
        LOG_ERROR("Load checkpoint failed. "
                  "which group_id=%" PRIu32", machine_id=%d, dir=%s.",
                  config_->GetGroupId(), it->first, it->second.c_str());
        break;
      }
    }
  }
  ComfirmReceive(msg, res);
  return res;
}

void CheckpointReceiver::ComfirmReceive(
    const CheckpointMessage& msg, bool res) {
  CheckpointMessage* reply_msg = new CheckpointMessage();
  reply_msg->set_type(CHECKPOINT_COMFIRM);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_sequence_id(msg.sequence_id());
  reply_msg->set_flag(res);
  messager_->SendMessage(msg.node_id(), messager_->PackMessage(reply_msg));
  if (!res && msg.node_id() == sender_node_id_) {
    Reset();
  }
}

void CheckpointReceiver::Reset() {
  sender_node_id_ = 0;
  sequence_id_ = 0;
  dirs_.clear();
}

}  // namespace skywalker
