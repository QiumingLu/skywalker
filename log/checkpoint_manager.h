// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
#define SKYWALKER_LOG_CHECKPOINT_MANAGER_H_

#include "log/checkpoint_receiver.h"
#include "log/checkpoint_sender.h"

namespace skywalker {

class Config;

class CheckpointManager {
 public:
  explicit CheckpointManager(Config* config);
  ~CheckpointManager();

  bool SendCheckpoint(uint64_t node_id);
  bool ReceiveCheckpoint(const CheckpointMessage& message);

 private:
  Config* config_;

  CheckpointSender sender_;
  CheckpointReceiver receiver_;

  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
