// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_receiver.h"

#include <unistd.h>
#include <vector>

#include "log/checkpoint_manager.h"
#include "paxos/config.h"
#include "skywalker/file.h"
#include "skywalker/logging.h"

namespace skywalker {

CheckpointReceiver::CheckpointReceiver(Config* config,
                                       CheckpointManager* manager)
    : config_(config), manager_(manager) {}

CheckpointReceiver::~CheckpointReceiver() {}

bool CheckpointReceiver::BeginToReceive(const CheckpointMessage& msg) {
  sender_node_id_ = msg.node_id();
  sequence_id_ = 0;
  dirs_.clear();
  bool res = DeleteTempCheckpoint(config_);
  return ComfirmReceive(msg, res);
}

bool CheckpointReceiver::ReceiveCheckpoint(const CheckpointMessage& msg) {
  LOG_DEBUG("Group %u - instance(id=%llu) sequence_id(%d==%d)",
            config_->GetGroupId(), (unsigned long long)msg.instance_id(),
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

  std::string dir = GetTempCheckpointPath(
      config_, msg.instance_id(), msg.machine_id());
  if (dirs_.find(msg.machine_id()) == dirs_.end()) {
    FileManager::Instance()->CreateDir(dir);
    bool exits = FileManager::Instance()->FileExists(dir);
    if (!exits) {
      LOG_ERROR("Group %u - create checkpoint dir=%s failed.",
                config_->GetGroupId(), dir.c_str());
      return false;
    }
    dirs_.insert(msg.machine_id());
  }

  WritableFile* file;
  std::string fname = dir + "/" + msg.file();
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

  if (config_->GetMachineManager()->UpdateCheckpoint(msg.instance_id())) {
    LOG_INFO("Group %u - instance(id=%llu) update checkpoint successful!",
             config_->GetGroupId(), (unsigned long long)msg.instance_id());
    LOG_WARN("Killing the process now...");
    // FIXME
    _exit(2);
  }
  return false;
}

bool CheckpointReceiver::ComfirmReceive(const CheckpointMessage& msg,
                                        bool res) {
  Content content;
  content.set_type(CHECKPOINT_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  CheckpointMessage* reply_msg = content.mutable_checkpoint_msg();
  reply_msg->set_type(CHECKPOINT_COMFIRM);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_sequence_id(msg.sequence_id());
  reply_msg->set_flag(res);
  config_->GetMessager()->SendMessage(msg.node_id(), content);
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
