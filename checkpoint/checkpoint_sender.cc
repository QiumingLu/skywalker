// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "checkpoint/checkpoint_sender.h"

namespace skywalker {

CheckpointSender::CheckpointSender() {
}

bool CheckpointSender::LockCheckpoint() {
  std::vector<StateMachine*> temp;
  bool res = true;
  for (auto& machine : machines_) {
    res = machine->LockCheckpoint();
    if (!res) {
      break;
    }
    temp.push_back(machine);
  }
  if (!res) {
    for (auto& machine : temp) {
      machine->UnLockCheckpoint();
    }
  }
  return res;
}

void CheckpointSender::UnLockCheckpoint() {
  for (auto& machine : machines_) {
    machine->UnLockCheckpoint();
  }
}

bool CheckpointSender::SendCheckpoint(uint32_t group_id) {
  bool res = LockCheckpoint();
  if (res) {
    for (auto& machine : machines_) {
      machine->GetCheckpoint(group_id);
    }
    UnLockCheckpoint();
  }
  return res;
}

}  // namespace skywalker
