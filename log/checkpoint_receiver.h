#ifndef SKYWALKER_LOG_CHECKPOINT_RECEIVER_H_
#define SKYWALKER_LOG_CHECKPOINT_RECEIVER_H_

#include <map>

#include "paxos/config.h"
#include "skywalker/checkpoint.h"

namespace skywalker {

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
  Checkpoint* checkpoint_;
  Messager* messager_;
  CheckpointManager* manager_;

  uint64_t sender_node_id_;
  uint64_t sequence_id_;
  std::map<int, std::string> dirs_;

  // No copying allowed
  CheckpointReceiver(const CheckpointReceiver&);
  void operator=(const CheckpointReceiver&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_RECEIVER_H__
