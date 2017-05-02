#ifndef JOURNEY_CHECKPOINT_IMPL_H_
#define JOURNEY_CHECKPOINT_IMPL_H_

#include <skywalker/checkpoint.h>

namespace journey {

class CheckpointImpl : public skywalker::Checkpoint {
 public:
  CheckpointImpl() { };
  virtual ~CheckpointImpl() { };

  virtual uint64_t GetCheckpointInstanceId(uint32_t group_id) {
    return -1;
  }

  virtual bool LockCheckpoint(uint32_t group_id) {
    return true;
  }

  virtual bool UnLockCheckpoint(uint32_t group_id) {
    return true;
  }

  virtual bool GetCheckpoint(
      uint32_t group_id, int machine_id,
      std::string* dirs, std::vector<std::string>* files) {
    return true;
  }

  virtual bool LoadCheckpoint(
      uint32_t group_id, int machine_id,
      const std::string& dir, const std::vector<std::string>& files) {
    return true;
  }

 private:
  // No copying allowed
  CheckpointImpl(const CheckpointImpl&);
  void operator=(const CheckpointImpl&);
};

}  // namespace journey

#endif  // JOURNEY_CHECKPOINT_IMPL_H_
