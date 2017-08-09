// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/checkpoint.h"

namespace skywalker {

uint64_t Checkpoint::GetCheckpointInstanceId(uint32_t group_id) { return -1; }

bool Checkpoint::LockCheckpoint(uint32_t group_id) { return true; }

bool Checkpoint::UnLockCheckpoint(uint32_t group_id) { return true; }

bool Checkpoint::GetCheckpoint(uint32_t group_id, uint32_t machine_id,
                               std::string* dir,
                               std::vector<std::string>* files) {
  return true;
}

bool Checkpoint::LoadCheckpoint(uint32_t group_id, uint32_t machine_id,
                                const std::string& dir,
                                const std::vector<std::string>& files) {
  return true;
}

}  // namespace skywalker
