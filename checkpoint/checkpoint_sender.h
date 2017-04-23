// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_CHECKPOINT_CHECKPOINT_SENDER_H_
#define SKYWALKER_CHECKPOINT_CHECKPOINT_SENDER_H_

#include <stdint.h>
#include <vector>

#include "skywalker/state_machine.h"

namespace skywalker {

class CheckpointSender {
 public:
  CheckpointSender();

  bool SendCheckpoint(uint32_t group_id);

 private:
  bool LockCheckpoint();
  void UnLockCheckpoint();

  std::vector<StateMachine*> machines_;

  // No copying allowed
  CheckpointSender(const CheckpointSender&);
  void operator=(const CheckpointSender&);
};

}  // namespace skywalker

#endif  // SKYWALKER_CHECKPOINT_CHECKPOINT_SENDER_H_
