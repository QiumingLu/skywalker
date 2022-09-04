// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/checkpoint_manager.h"
#include "paxos/config.h"
#include "skywalker/checkpoint.h"
#include "skywalker/logging.h"

namespace skywalker {

CheckpointManager::CheckpointManager(Config* config)
    : config_(config), sender_(config, this), receiver_(config, this) {}

CheckpointManager::~CheckpointManager() {}

bool CheckpointManager::SendCheckpoint(uint64_t node_id) {
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
      LOG_ERROR("Group %u - receive an invalid checkpoint message.",
                config_->GetGroupId());
      break;
  }
  return res;
}

}  // namespace skywalker
