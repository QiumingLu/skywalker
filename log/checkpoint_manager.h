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

  bool SendCheckpoint();

 private:
  bool SendFile(const std::string& fname);

  Config* config_;
  Checkpoint* checkpoint_;
  FileManager* file_manager_;

  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_CHECKPOINT_MANAGER_H_
