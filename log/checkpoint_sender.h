#ifndef SKYWALKER_LOG_CHECKPOINT_SENDER_H_
#define SKYWALKER_LOG_CHECKPOINT_SENDER_H_

#include "util/mutex.h"
#include "paxos/config.h"
#include "skywalker/checkpoint.h"

namespace skywalker {

class CheckpointManager;

class CheckpointSender {
 public:
  CheckpointSender(Config* config, CheckpointManager* manager);
  ~CheckpointSender();

  bool SendCheckpoint(uint64_t node_id);
  void OnComfirmReceive(const CheckpointMessage& msg);

 private:
  static const int kBufferSize = 65536;

  void BeginToSend(uint64_t instance_id);
  bool SendCheckpointFiles(uint64_t instance_id);
  bool SendFile(uint64_t instance_id, int machine_id,
                const std::string& dir, const std::string& file);
  void EndToSend(uint64_t instance_id);

  bool CheckReceive();

  char buffer[kBufferSize];

  Config* config_;
  Checkpoint* checkpoint_;
  Messager* messager_;
  CheckpointManager* manager_;

  uint64_t receiver_node_id_;
  uint64_t sequence_id_;

  Mutex mutex_;
  Condition cond_;
  uint64_t ack_sequence_id_;
  bool error_;

  // No copying allowed
  CheckpointSender(const CheckpointSender&);
  void operator=(const CheckpointSender&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_SENDER_H_
