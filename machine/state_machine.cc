// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/state_machine.h"

namespace skywalker {

uint64_t StateMachine::GetCheckpointInstanceId(uint32_t group_id) const {
  return -1;
}

bool StateMachine::BuildCheckpoint(uint32_t group_id, uint64_t instance_id,
                                   const std::string& value) {
  return true;
}

bool StateMachine::LockCheckpoint() {
  return true;
}

bool StateMachine::GetCheckpoint(uint32_t group_id) {
  return true;
}

bool StateMachine::UnLockCheckpoint() {
  return true;
}

bool StateMachine::LoadCheckpoint(uint32_t group_id) {
  return true;
}

}  // namespace skywalker
