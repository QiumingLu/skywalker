#include "paxos/transfer.h"
#include "paxos/config.h"
#include "util/mutexlock.h"
#include "skywalker/logging.h"

namespace skywalker {

Transfer::Transfer(Config* config)
    : config_(config),
      loop_(config_->GetLoop()),
      mutex_(),
      cond_(&mutex_),
      transfer_end_(false),
      instance_id_(0),
      value_(),
      context_(nullptr),
      success_(false) {
}

bool Transfer::NewValue(const Slice& value,
                        MachineContext* context,
                        uint64_t* new_instance_id) {
  transfer_end_ = false;
  instance_id_ = 0;
  value_ = value;
  context_ = context;
  success_ = false;
  loop_->NewValue(value);

  MutexLock lock(&mutex_);
  while (!transfer_end_) {
    cond_.Wait();
  }
  if (success_) {
    *new_instance_id = instance_id_;
  } else {
    SWLog(INFO,
          "Transfer::NewValue - handle new value(%s) timeout.\n",
          value.data());
  }
  return success_;
}

void Transfer::SetNowInstanceId(uint64_t instance_id) {
  MutexLock lock(&mutex_);
  instance_id_ = instance_id;
}

bool Transfer::IsMyProposal(uint64_t instance_id,
                            const Slice& learned_value,
                            MachineContext** context) const {
  if (!transfer_end_ &&
      instance_id_ == instance_id &&
      value_ == learned_value) {
    *context = context_;
    return true;
  }
  return false;
}

void Transfer::SetResult(bool success, uint64_t instance_id,
                         const Slice& value) {
  if(instance_id_ == instance_id) {
    MutexLock lock(&mutex_);
    success_ = success;
    if (value_ != value) {
      success_ = false;
    }
    transfer_end_ = true;
    cond_.Signal();
  }
}

}  // namespace skywalker
