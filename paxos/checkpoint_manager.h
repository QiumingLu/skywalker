// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_CHECKPOINT_MANAGER_H_
#define SKYWALKER_PAXOS_CHECKPOINT_MANAGER_H_

namespace skywalker {

class CheckpointManager {
 public:
  CheckpointManager();

 private:
  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_CHECKPOINT_MANAGER_H_
