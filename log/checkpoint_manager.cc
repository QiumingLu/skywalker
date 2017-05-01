// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_manager.h"
#include "util/mutexlock.h"
#include "util/timeops.h"
#include "skywalker/logging.h"

namespace skywalker {

CheckpointManager::CheckpointManager(Config* config)
    : config_(config),
      checkpoint_(config_->GetCheckpoint()),
      messager_(config_->GetMessager()),
      send_node_id_(0),
      sequence_id_(0),
      mutex_(),
      cond_(&mutex_),
      ack_sequence_id_(0) {
}

CheckpointManager::~CheckpointManager() {
}

uint64_t CheckpointManager::GetCheckpointInstanceId() const {
  uint64_t id = -1;
  if (config_->StateMachines().empty()) {
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
  send_node_id_ = node_id;
  sequence_id_ = 0;
  ack_sequence_id_ = 0;
  error_ = false;
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

  const std::vector<StateMachine*>& machines = config_->StateMachines();
  for (auto& machine : machines) {
    files.clear();
    res = checkpoint_->GetCheckpoint(config_->GetGroupId(),
                                     machine->machine_id(),
                                     &dir, &files);
    if (!res) {
      SWLog(ERROR, "Get checkpoint failed, "
            "which groud_id=%" PRIu32", machine_id=%d\n",
            config_->GetGroupId(), machine->machine_id());
      return res;
    }

    if (dir.empty()) {
      continue;
    }
    if (dir[dir.size() - 1] != '/') {
      dir += '/';
    }
    for (auto& file : files) {
      res = SendFile(node_id, instance_id, machine->machine_id(), dir, file);
      if (!res) {
        SWLog(ERROR, "Send file failed, file:%s%s\n", dir.c_str(), file.c_str());
        break;
      }
    }
  }
  return res;
}

bool CheckpointManager::SendFile(
    uint64_t node_id, uint64_t instance_id, int machine_id,
    const std::string& dir, const std::string& file) {
  std::string fname = dir + file;
  SequentialFile* seq_file;
  Status s = FileManager::Instance()->NewSequentialFile(fname, &seq_file);
  if (!s.ok()) {
    SWLog(ERROR, "%s\n", s.ToString().c_str());
    return false;
  }
  bool res = true;
  size_t offset = 0;
  while (res) {
    Slice fragmenet;
    s = seq_file->Read(kBufferSize, &fragmenet, buffer);
    if (!s.ok()) {
      res = false;
      SWLog(ERROR, "%s\n", s.ToString().c_str());
    }
    if (fragmenet.empty()) {
      break;
    }

    offset += fragmenet.size();

    CheckpointMessage* msg = new CheckpointMessage();
    msg->set_type(CHECKPOINT_FILE);
    msg->set_node_id(config_->GetNodeId());
    msg->set_sequence_id(sequence_id_++);
    msg->set_instance_id(instance_id);
    msg->set_machine_id(machine_id);
    msg->set_file(fname);
    msg->set_offset(offset);
    msg->set_data(fragmenet.data(), fragmenet.size());
    messager_->SendMessage(node_id, messager_->PackMessage(msg));
    res = CheckReceive();
  }
  delete seq_file;

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
  ComfirmReceive(msg, res);
  return res;
}

bool CheckpointManager::BeginToReceive(const CheckpointMessage& msg) {
  receive_node_id_ = msg.node_id();
  receive_sequence_id_ = 0;

  bool res = true;
  std::vector<std::string> dirs;
  FileManager::Instance()->GetChildren(config_->CheckpointPath(), &dirs);
  for (auto& dir : dirs) {
    std::string abs_dir = config_->CheckpointPath() + "/" + dir;
    std::vector<std::string> filenames;
    FileManager::Instance()->GetChildren(abs_dir, &filenames);
    if (filenames.empty()) {
      continue;
    }
    for (auto& filename : filenames) {
      Status del = FileManager::Instance()->DeleteFile(abs_dir + "/" + filename);
      if (!del.ok()) {
        res = false;
      }
    }
    FileManager::Instance()->DeleteDir(abs_dir);
  }
  return res;
}

bool CheckpointManager::ReceiveFiles(const CheckpointMessage& msg) {
  if (msg.node_id() != receive_node_id_) {
    return false;
  }

  if (msg.sequence_id() ==  receive_sequence_id_) {
    return true;
  }

  if (msg.sequence_id() != receive_sequence_id_ + 1) {
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
      ++receive_sequence_id_;
    }
    delete file;
  }
  return s.ok() ? true : false;
}

bool CheckpointManager::EndToReceive(const CheckpointMessage& msg) {
  if (msg.node_id() != receive_node_id_ ||
      msg.sequence_id() != receive_sequence_id_ + 1) {
    return false;
  }
  const std::vector<StateMachine*>& machines(config_->StateMachines());
  bool res = true;
  for (auto machine : machines) {
    if (dirs_.find(machine->machine_id()) != dirs_.end()) {
      std::vector<std::string> files;
      Status s = FileManager::Instance()->GetChildren(
          dirs_[machine->machine_id()], &files);
      if (!s.ok()) {
        SWLog(ERROR, "%s\n", s.ToString().c_str());
        res = false;
        break;
      }
      if (files.empty()) {
        continue;
      }
      res = checkpoint_->LoadCheckpoint(
          config_->GetGroupId(), machine->machine_id(),
          dirs_[machine->machine_id()], files);
      if (!res) {
        SWLog(ERROR, "Load checkpoint failed. "
              "group_id=%" PRIu32", machine_id=%d, dir=%s.\n",
              config_->GetGroupId(), machine->machine_id(),
              dirs_[machine->machine_id()].c_str());
        break;
      }
    }
  }

  return res;
}

void CheckpointManager::ComfirmReceive(const CheckpointMessage& msg, bool res) {
  CheckpointMessage* reply_msg = new CheckpointMessage();
  reply_msg->set_type(CHECKPOINT_COMFIRM);
  reply_msg->set_node_id(config_->GetNodeId());
  reply_msg->set_sequence_id(msg.sequence_id());
  reply_msg->set_flag(res);
  messager_->SendMessage(msg.node_id(), messager_->PackMessage(reply_msg));
  if (!res) {
    receive_node_id_ = 0;
    receive_sequence_id_ = 0;
    dirs_.clear();
  }
}

void CheckpointManager::OnComfirmReceive(const CheckpointMessage& msg) {
  MutexLock lock(&mutex_);
  if (msg.node_id() == send_node_id_ && msg.sequence_id() == ack_sequence_id_) {
    ++ack_sequence_id_;
    error_ = msg.flag();
    cond_.Signal();
  } else {
    SWLog(ERROR, "receive confirm message node_id = %" PRIu64", "
          "sequence_id=%" PRIu64", but send_node_id_=%" PRIu64", "
          "ack_sequence_id_=%" PRIu64".",
          msg.node_id(), msg.sequence_id(), send_node_id_, ack_sequence_id_);
  }
}

bool CheckpointManager::CheckReceive() {
  bool res = true;
  MutexLock lock(&mutex_);
  while (!error_ && sequence_id_ > ack_sequence_id_ + 10 && res) {
    res = cond_.Wait(25 * 1000 * 1000);
    if (!res) {
      SWLog(ERROR, "receive comfirm message timeout!\n");
    }
  }
  if (error_) {
    res = false;
  }
  return res;
}

}  // namespace skywalker
