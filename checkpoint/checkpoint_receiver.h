// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_CHECKPOINT_CHECKPOINT_RECEIVER_H_
#define SKYWALKER_CHECKPOINT_CHECKPOINT_RECEIVER_H_

namespace skywalker {

class CheckpointReceiver {
 public:
  CheckpointReceiver();

 private:
  // No copying allowed
  CheckpointReceiver(const CheckpointReceiver&);
  void operator=(const CheckpointReceiver&);
};

}  // namespace skywalker

#endif  // SKYWALKER_CHECKPOINT_CHECKPOINT_RECEIVER_H_
