#ifndef SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
#define SKYWALKER_LOG_CHECKPOINT_MANAGER_H_

#include "util/mutex.h"
#include "paxos/config.h"
#include "skywalker/checkpoint.h"
#include "util/file.h"

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
  bool SendFile(uint64_t node_id, uint64_t instance_id,
                const std::string& fname);
  void EndToSend(uint64_t node_id, uint64_t instance_id);

  bool BeginToReceive(const CheckpointMessage& msg);
  bool ReceiveFiles(const CheckpointMessage& msg);
  bool EndToReceive(const CheckpointMessage& msg);

  void ComfirmReceive(bool res);
  void OnComfirmReceive(const CheckpointMessage& msg);

  char buffer[kBufferSize];

  Config* config_;
  Checkpoint* checkpoint_;
  Messager* messager_;
  FileManager* file_manager_;

  // for send
  uint64_t sequence_id_;

  // for receive
  uint64_t last_received_sender_node_id_;
  uint64_t last_received_sequence_id_;

  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
