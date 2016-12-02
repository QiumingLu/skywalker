#include "voyager/paxos/transfer.h"
#include "voyager/port/mutexlock.h"
#include "voyager/util/logging.h"

namespace voyager {
namespace paxos {

Transfer::Transfer(Config* config, RunLoop* loop)
    : config_(config),
      loop_(loop),
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
  bool res = true;
  bool wait = true;

  port::MutexLock lock(&mutex_);
  while (!transfer_end_ && wait) {
    res = cond_.Wait(5000);
    wait = false;
  }
  if (res) {
    if (success_) {
      *new_instance_id = instance_id_;
    }
  } else {
    VOYAGER_LOG(INFO) << "Transfer::NewValue - handle new value("
                      << value <<") timeout.";
  }
  return success_;
}

void Transfer::SetNowInstanceId(uint64_t instance_id) {
  port::MutexLock lock(&mutex_);
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
    port::MutexLock lock(&mutex_);
    success_ = success;
    if (value_ != value) {
      success_ = false;
    }
    transfer_end_ = true;
    cond_.Signal();
  }
}

}  // namespace paxos
}  // namespace voyager
