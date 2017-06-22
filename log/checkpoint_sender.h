// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_CHECKPOINT_SENDER_H_
#define SKYWALKER_LOG_CHECKPOINT_SENDER_H_

#include <string>

#include "proto/paxos.pb.h"
#include "util/mutex.h"

namespace skywalker {

class Config;
class CheckpointManager;

class CheckpointSender {
 public:
  CheckpointSender(Config* config, CheckpointManager* manager);
  ~CheckpointSender();

  bool SendCheckpoint(uint64_t node_id);
  void OnComfirmReceive(const CheckpointMessage& msg);

 private:
  static const int kBufferSize = 65535;

  void BeginToSend(uint64_t instance_id);
  bool SendCheckpointFiles(uint64_t instance_id);
  bool SendFile(uint64_t instance_id, int machine_id, const std::string& dir,
                const std::string& file);
  void EndToSend(uint64_t instance_id);

  bool CheckReceive();

  char buffer[kBufferSize];

  Config* config_;
  CheckpointManager* manager_;

  uint64_t receiver_node_id_;
  int sequence_id_;

  Mutex mutex_;
  Condition cond_;
  int ack_sequence_id_;
  bool flag_;

  // No copying allowed
  CheckpointSender(const CheckpointSender&);
  void operator=(const CheckpointSender&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_SENDER_H_
