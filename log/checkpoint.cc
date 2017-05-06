#include "skywalker/checkpoint.h"

namespace skywalker {

uint64_t Checkpoint::GetCheckpointInstanceId(uint32_t group_id) {
  return -1;
}

bool Checkpoint::LockCheckpoint(uint32_t group_id) {
  return true;
}

bool Checkpoint::UnLockCheckpoint(uint32_t group_id) {
  return true;
}

bool Checkpoint::GetCheckpoint(
    uint32_t group_id, int machine_id,
    std::string* dir, std::vector<std::string>* files) {
  return true;
}

bool Checkpoint::LoadCheckpoint(
      uint32_t group_id, int machine_id,
      const std::string& dir, const std::vector<std::string>& files) {
  return true;
}

}  // namespace skywalker
