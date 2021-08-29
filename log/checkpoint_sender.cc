// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_sender.h"

#include <vector>

#include "log/checkpoint_manager.h"
#include "paxos/config.h"
#include "skywalker/file.h"
#include "skywalker/logging.h"

namespace skywalker {

CheckpointSender::CheckpointSender(Config* config, CheckpointManager* manager)
    : config_(config),
      manager_(manager),
      receiver_node_id_(0),
      sequence_id_(0),
      ack_sequence_id_(0),
      flag_(true) {}

CheckpointSender::~CheckpointSender() {}

bool CheckpointSender::SendCheckpoint(uint64_t node_id) {
  receiver_node_id_ = node_id;
  sequence_id_ = 0;
  ack_sequence_id_ = 0;
  flag_ = true;
  bool res = config_->GetCheckpoint()->LockCheckpoint(config_->GetGroupId());
  if (res) {
    uint64_t instance_id = manager_->GetCheckpointInstanceId();
    BeginToSend(instance_id);

    res = SendCheckpointFiles(instance_id);

    if (res) {
      EndToSend(instance_id);
    } else {
      LOG_ERROR("Group %u - send checkpoint failed.", config_->GetGroupId());
    }
    config_->GetCheckpoint()->UnLockCheckpoint(config_->GetGroupId());
  } else {
    LOG_WARN("Group %u - lock checkpoint failed.", config_->GetGroupId());
  }
  return res;
}

void CheckpointSender::BeginToSend(uint64_t instance_id) {
  Content content;
  content.set_type(CHECKPOINT_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  CheckpointMessage* begin = content.mutable_checkpoint_msg();
  begin->set_type(CHECKPOINT_BEGIN);
  begin->set_node_id(config_->GetNodeId());
  begin->set_instance_id(instance_id);
  begin->set_sequence_id(sequence_id_++);
  config_->GetMessager()->SendMessage(receiver_node_id_, content);
}

bool CheckpointSender::SendCheckpointFiles(uint64_t instance_id) {
  bool res = true;
  std::string dir;
  std::vector<std::string> files;

  const std::vector<StateMachine*>& machines = config_->GetStateMachines();
  for (auto& machine : machines) {
    files.clear();
    res = config_->GetCheckpoint()->GetCheckpoint(
        config_->GetGroupId(), machine->machine_id(), &dir, &files);
    if (!res) {
      LOG_ERROR("Group %u - get checkpoint failed, the machine_id=%d.",
                config_->GetGroupId(), machine->machine_id());
      return res;
    }

    if (dir.empty() || files.empty()) {
      continue;
    }
    if (dir[dir.size() - 1] != '/') {
      dir += '/';
    }
    for (auto& file : files) {
      res = SendFile(instance_id, machine->machine_id(), dir, file);
      if (!res) {
        LOG_ERROR("Group %u - send file failed, the file=%s%s.",
                  config_->GetGroupId(), dir.c_str(), file.c_str());
        break;
      }
    }
  }
  return res;
}

bool CheckpointSender::SendFile(uint64_t instance_id, int machine_id,
                                const std::string& dir,
                                const std::string& file) {
  std::string fname = dir + file;
  SequentialFile* seq_file;
  Status s = FileManager::Instance()->NewSequentialFile(fname, &seq_file);
  if (!s.ok()) {
    LOG_ERROR("Group %u - %s", config_->GetGroupId(), s.ToString().c_str());
    return false;
  }
  bool res = true;
  size_t offset = 0;

  Content content;
  content.set_type(CHECKPOINT_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  CheckpointMessage* msg = content.mutable_checkpoint_msg();
  msg->set_type(CHECKPOINT_FILE);
  msg->set_node_id(config_->GetNodeId());
  msg->set_instance_id(instance_id);
  msg->set_machine_id(machine_id);
  msg->set_file(file);

  while (res) {
    Slice fragmenet;
    s = seq_file->Read(kBufferSize, &fragmenet, buffer_);
    if (!s.ok()) {
      res = false;
      LOG_ERROR("Group %u - %s", config_->GetGroupId(), s.ToString().c_str());
      break;
    }
    if (fragmenet.empty()) {
      break;
    }

    offset += fragmenet.size();
    msg->set_sequence_id(sequence_id_++);
    msg->set_offset(offset);
    msg->set_data(fragmenet.data(), fragmenet.size());
    config_->GetMessager()->SendMessage(receiver_node_id_, content);
    res = CheckReceive();
  }
  delete seq_file;

  return res;
}

void CheckpointSender::EndToSend(uint64_t instance_id) {
  Content content;
  content.set_type(CHECKPOINT_MESSAGE);
  content.set_group_id(config_->GetGroupId());
  CheckpointMessage* end = content.mutable_checkpoint_msg();
  end->set_type(CHECKPOINT_END);
  end->set_node_id(config_->GetNodeId());
  end->set_instance_id(instance_id);
  end->set_sequence_id(sequence_id_);
  config_->GetMessager()->SendMessage(receiver_node_id_, content);
}

void CheckpointSender::OnComfirmReceive(const CheckpointMessage& msg) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (msg.node_id() == receiver_node_id_ &&
      msg.sequence_id() == ack_sequence_id_) {
    ++ack_sequence_id_;
    flag_ = msg.flag();
    cond_.notify_one();
  } else {
    LOG_WARN(
        "Group %u - receive a confirm message, "
        "which node_id = %llu, sequence_id=%d, "
        "but my sender_node_id_=%llu, ack_sequence_id_=%d.",
        config_->GetGroupId(), (unsigned long long)msg.node_id(),
        msg.sequence_id(), (unsigned long long)receiver_node_id_,
        ack_sequence_id_);
  }
}

bool CheckpointSender::CheckReceive() {
  bool res = true;
  std::unique_lock<std::mutex> lock(mutex_);
  while (flag_ && (sequence_id_ > ack_sequence_id_ + 16)) {
    std::cv_status cs =
        cond_.wait_for(lock, std::chrono::microseconds(10 * 1000 * 1000));
    if (cs == std::cv_status::timeout) {
      LOG_ERROR("Group %u - receive comfirm message timeout!",
                config_->GetGroupId());
      break;
    }
  }
  if (!flag_) {
    LOG_ERROR("Group %u - error happen.", config_->GetGroupId());
    res = false;
  }
  return res;
}

}  // namespace skywalker
