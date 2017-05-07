// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_CHECKPOINT_RECEIVER_H_
#define SKYWALKER_LOG_CHECKPOINT_RECEIVER_H_

#include <map>
#include <string>

#include "proto/paxos.pb.h"

namespace skywalker {

class Config;
class CheckpointManager;

class CheckpointReceiver {
 public:
  CheckpointReceiver(Config* config, CheckpointManager* manager);
  ~CheckpointReceiver();

  bool BeginToReceive(const CheckpointMessage& msg);

  bool ReceiveCheckpoint(const CheckpointMessage& msg);

  bool EndToReceive(const CheckpointMessage& msg);

 private:
  bool ReceiveFiles(const CheckpointMessage& msg);
  bool ComfirmReceive(const CheckpointMessage& msg, bool res);
  void Reset();

  Config* config_;
  CheckpointManager* manager_;

  uint64_t sender_node_id_;
  int sequence_id_;
  std::map<int, std::string> dirs_;

  // No copying allowed
  CheckpointReceiver(const CheckpointReceiver&);
  void operator=(const CheckpointReceiver&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_RECEIVER_H__
