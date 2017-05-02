// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_sender.h"
#include "log/checkpoint_manager.h"
#include "util/mutexlock.h"
#include "skywalker/logging.h"
#include "skywalker/file.h"

namespace skywalker {

CheckpointSender::CheckpointSender(Config* config, CheckpointManager* manager)
    : config_(config),
      checkpoint_(config_->GetCheckpoint()),
      messager_(config_->GetMessager()),
      manager_(manager),
      receiver_node_id_(0),
      sequence_id_(0),
      mutex_(),
      cond_(&mutex_),
      ack_sequence_id_(0),
      error_(false) {
}

CheckpointSender::~CheckpointSender() {
}

bool CheckpointSender::SendCheckpoint(uint64_t node_id) {
  receiver_node_id_ = node_id;
  sequence_id_ = 0;
  ack_sequence_id_ = 0;
  error_ = false;
  bool res = checkpoint_->LockCheckpoint(config_->GetGroupId());
  if (res) {
    uint64_t instance_id = manager_->GetCheckpointInstanceId();
    BeginToSend(instance_id);

    res = SendCheckpointFiles(instance_id);

    if (res) {
      EndToSend(instance_id);
    } else {
      LOG_ERROR("Send checkpoint files failed.");
    }
    checkpoint_->UnLockCheckpoint(config_->GetGroupId());
  } else {
    LOG_WARN("Lock checkpoint failed.");
  }
  return res;
}

void CheckpointSender::BeginToSend(uint64_t instance_id) {
  CheckpointMessage* begin = new CheckpointMessage();
  begin->set_type(CHECKPOINT_BEGIN);
  begin->set_node_id(config_->GetNodeId());
  begin->set_sequence_id(sequence_id_++);
  begin->set_instance_id(instance_id);
  messager_->SendMessage(receiver_node_id_, messager_->PackMessage(begin));
}

bool CheckpointSender::SendCheckpointFiles(uint64_t instance_id) {
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
      LOG_ERROR(
          "Get checkpoint failed, which groud_id=%" PRIu32", machine_id=%d.",
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
      res = SendFile(instance_id, machine->machine_id(), dir, file);
      if (!res) {
        LOG_ERROR("Send file failed, file:%s%s.", dir.c_str(), file.c_str());
        break;
      }
    }
  }
  return res;
}

bool CheckpointSender::SendFile(
    uint64_t instance_id, int machine_id,
    const std::string& dir, const std::string& file) {
  std::string fname = dir + file;
  SequentialFile* seq_file;
  Status s = FileManager::Instance()->NewSequentialFile(fname, &seq_file);
  if (!s.ok()) {
    LOG_ERROR("%s", s.ToString().c_str());
    return false;
  }
  bool res = true;
  size_t offset = 0;
  while (res) {
    Slice fragmenet;
    s = seq_file->Read(kBufferSize, &fragmenet, buffer);
    if (!s.ok()) {
      res = false;
      LOG_ERROR("%s", s.ToString().c_str());
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
    msg->set_file(file);
    msg->set_offset(offset);
    msg->set_data(fragmenet.data(), fragmenet.size());
    messager_->SendMessage(receiver_node_id_, messager_->PackMessage(msg));
    res = CheckReceive();
  }
  delete seq_file;

  return res;
}

void CheckpointSender::EndToSend(uint64_t instance_id) {
  CheckpointMessage* end =  new CheckpointMessage();
  end->set_type(CHECKPOINT_END);
  end->set_node_id(config_->GetNodeId());
  end->set_sequence_id(sequence_id_++);
  end->set_instance_id(instance_id);
  messager_->SendMessage(receiver_node_id_, messager_->PackMessage(end));
}

void CheckpointSender::OnComfirmReceive(const CheckpointMessage& msg) {
  MutexLock lock(&mutex_);
  if (msg.node_id() == receiver_node_id_ &&
      msg.sequence_id() == ack_sequence_id_) {
    ++ack_sequence_id_;
    error_ = msg.flag();
    cond_.Signal();
  } else {
    LOG_WARN(
        "receive confirm message, which node_id = %" PRIu64", "
        "sequence_id=%" PRIu64", but send_node_id_=%" PRIu64", "
        "ack_sequence_id_=%" PRIu64".",
        msg.node_id(), msg.sequence_id(), receiver_node_id_, ack_sequence_id_);
  }
}

bool CheckpointSender::CheckReceive() {
  bool res = true;
  MutexLock lock(&mutex_);
  while (!error_ && (sequence_id_ > ack_sequence_id_ + 10) && res) {
    res = cond_.Wait(25 * 1000 * 1000);
    if (!res) {
      LOG_ERROR("receive comfirm message timeout!");
    }
  }
  if (error_) {
    res = false;
  }
  return res;
}

}  // namespace skywalker
