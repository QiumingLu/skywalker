// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_manager.h"
#include "skywalker/logging.h"

namespace skywalker {

CheckpointManager::CheckpointManager(Config* config)
    : config_(config),
      checkpoint_(config_->GetCheckpoint()),
      sender_(config, this),
      receiver_(config, this) {
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
  // TODO
  return sender_.SendCheckpoint(node_id);
}

bool CheckpointManager::ReceiveCheckpoint(const CheckpointMessage& msg) {
  bool res = true;
  switch (msg.type()) {
    case CHECKPOINT_BEGIN:
      res = receiver_.BeginToReceive(msg);
      break;
    case CHECKPOINT_FILE:
      res = receiver_.ReceiveCheckpoint(msg);
      break;
    case CHECKPOINT_END:
      res = receiver_.EndToReceive(msg);
      break;
    case CHECKPOINT_COMFIRM:
      sender_.OnComfirmReceive(msg);
      break;
    default:
      res = false;
      LOG_ERROR("Error checkpoint message type.");
      break;
  }
  return res;
}

}  // namespace skywalker
