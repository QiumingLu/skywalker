#ifndef SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
#define SKYWALKER_LOG_CHECKPOINT_MANAGER_H_

#include "log/checkpoint_sender.h"
#include "log/checkpoint_receiver.h"
#include "paxos/config.h"
#include "skywalker/checkpoint.h"

namespace skywalker {

class CheckpointManager {
 public:
  CheckpointManager(Config* config);
  ~CheckpointManager();

  uint64_t GetCheckpointInstanceId() const;

  bool SendCheckpoint(uint64_t node_id);
  bool ReceiveCheckpoint(const CheckpointMessage& message);

 private:
  Config* config_;
  Checkpoint* checkpoint_;

  std::set<uint64_t> nodes_;

  CheckpointSender sender_;
  CheckpointReceiver receiver_;

  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
