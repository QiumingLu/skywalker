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
      messager_(config_->GetMessager()),
      file_manager_(new FileManager()),
      sequence_id_(0) {
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

bool CheckpointManager::SendCheckpoint(uint64_t node_id) {
  bool res = checkpoint_->LockCheckpoint(config_->GetGroupId());
  if (res) {
    uint64_t instance_id = GetCheckpointInstanceId();
    BeginToSend(node_id, instance_id);
    res = SendCheckpointFiles(node_id, instance_id);
    if (res) {
      EndToSend(node_id, instance_id);
    }
    checkpoint_->UnLockCheckpoint(config_->GetGroupId());
  }
  return res;
}

void CheckpointManager::BeginToSend(
    uint64_t node_id, uint64_t instance_id) {
  CheckpointMessage* begin = new CheckpointMessage();
  begin->set_type(CHECKPOINT_BEGIN);
  begin->set_node_id(config_->GetNodeId());
  begin->set_sequence_id(sequence_id_++);
  begin->set_instance_id(instance_id);
  messager_->SendMessage(node_id, messager_->PackMessage(begin));
}

bool CheckpointManager::SendCheckpointFiles(
    uint64_t node_id, uint64_t instance_id) {
  bool res = true;
  std::string dir;
  std::vector<std::string> files;
  res = checkpoint_->GetCheckpoint(config_->GetGroupId(), &dir, &files);
  if (!res) {
    SWLog(ERROR, "Get checkpoint failed, which groud_id=%" PRIu32"\n",
          config_->GetGroupId());
    return res;
  }
  if (dir.empty()) {
    return res;
  }
  if (dir[dir.size() - 1] != '/') {
    dir += '/';
  }
  for (auto& file : files) {
    std::string fname = dir + file;
    res = SendFile(node_id, instance_id, fname);
    if (!res) {
      SWLog(ERROR, "Send file failed, file:%s\n", fname.c_str());
      break;
    }
  }
  return res;
}

bool CheckpointManager::SendFile(uint64_t node_id, uint64_t instance_id,
                                 const std::string& fname) {
  SequentialFile* file;
  Status s = file_manager_->NewSequentialFile(fname, &file);
  if (!s.ok()) {
    SWLog(ERROR, "%s\n", s.ToString().c_str());
    return false;
  }
  bool res = true;
  while (true) {
    Slice fragmenet;
    s = file->Read(kBufferSize, &fragmenet, buffer);
    if (!s.ok()) {
      res = false;
      SWLog(ERROR, "%s\n", s.ToString().c_str());
      break;
    }
    if (fragmenet.empty()) {
      break;
    }

    CheckpointMessage* msg = new CheckpointMessage();
    msg->set_type(CHECKPOINT_FILE);
    msg->set_node_id(config_->GetNodeId());
    msg->set_sequence_id(sequence_id_++);
    msg->set_instance_id(instance_id);
    msg->set_file(fname);
    msg->set_offset(fragmenet.size());
    msg->set_data(fragmenet.data(), fragmenet.size());
    messager_->SendMessage(node_id, messager_->PackMessage(msg));
  }
  delete file;

  return res;
}

void CheckpointManager::EndToSend(
    uint64_t node_id, uint64_t instance_id) {
  CheckpointMessage* end =  new CheckpointMessage();
  end->set_type(CHECKPOINT_END);
  end->set_node_id(config_->GetNodeId());
  end->set_sequence_id(sequence_id_++);
  end->set_instance_id(instance_id);
  messager_->SendMessage(node_id, messager_->PackMessage(end));
}

bool CheckpointManager::ReceiveCheckpoint(const CheckpointMessage& msg) {
  bool res = true;
  switch (msg.type()) {
    case CHECKPOINT_BEGIN:
      res = BeginToReceive(msg);
      break;
    case CHECKPOINT_FILE:
      res = ReceiveFiles(msg);
      break;
    case CHECKPOINT_END:
      res = EndToReceive(msg);
      break;
    case CHECKPOINT_COMFIRM:
      OnComfirmReceive(msg);
      break;
    default:
      res = false;
      SWLog(ERROR, "Error checkpoint message type\n");
      break;
  }
  ComfirmReceive(res);
  return res;
}

bool CheckpointManager::BeginToReceive(const CheckpointMessage& msg) {
  last_received_sender_node_id_ = msg.node_id();
  last_received_sequence_id_ = 0;
  return true;
}

bool CheckpointManager::ReceiveFiles(const CheckpointMessage& msg) {
  if (msg.node_id() != last_received_sender_node_id_) {
    return false;
  }
  return true;
}

bool CheckpointManager::EndToReceive(const CheckpointMessage& msg) {
  return true;
}

void CheckpointManager::ComfirmReceive(bool res) {
}

void CheckpointManager::OnComfirmReceive(const CheckpointMessage& msg) {
}

}  // namespace skywalker
