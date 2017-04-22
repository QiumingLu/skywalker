// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_CHECKPOINT_MANAGER_H_
#define SKYWALKER_PAXOS_CHECKPOINT_MANAGER_H_

#include "paxos/config.h"
#include "paxos/state_machine_manager.h"

namespace skywalker {

class CheckpointManager {
 public:
  CheckpointManager(Config* config, StateMachineManager* manager);

  bool Init(uint64_t instance_id);

 private:
  bool ReplayLog(uint64_t from, uint64_t to);

  Config* config_;
  StateMachineManager* state_machine_manager_;
  uint64_t min_chosen_id_;

  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_CHECKPOINT_MANAGER_H_
