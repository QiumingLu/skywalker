#ifndef SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
#define SKYWALKER_LOG_CHECKPOINT_MANAGER_H_

#include <memory>

#include "util/mutex.h"
#include "paxos/config.h"
#include "skywalker/file.h"
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
  static const int kBufferSize = 65536;

  void BeginToSend(uint64_t node_id, uint64_t instance_id);
  bool SendCheckpointFiles(uint64_t node_id, uint64_t instance_id);
  bool SendFile(uint64_t node_id, uint64_t instance_id, int machine_id,
                const std::string& dir, const std::string& file);
  void EndToSend(uint64_t node_id, uint64_t instance_id);

  bool BeginToReceive(const CheckpointMessage& msg);
  bool ReceiveFiles(const CheckpointMessage& msg);
  bool EndToReceive(const CheckpointMessage& msg);

  void ComfirmReceive(const CheckpointMessage& msg, bool res);
  void OnComfirmReceive(const CheckpointMessage& msg);
  bool CheckReceive();

  char buffer[kBufferSize];

  Config* config_;
  Checkpoint* checkpoint_;
  Messager* messager_;

  // for send
  uint64_t send_node_id_;
  uint64_t sequence_id_;

  Mutex mutex_;
  Condition cond_;
  uint64_t ack_sequence_id_;
  bool error_;

  // for receive
  uint64_t receive_node_id_;
  uint64_t receive_sequence_id_;
  std::map<int, std::string> dirs_;

  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
