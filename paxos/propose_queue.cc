#include "paxos/propose_queue.h"

namespace skywalker {

ProposeQueue::ProposeQueue(Config* config)
    : config_(config),
      exit_(false),
      thread_(),
      mutex_(),
      cond_(&mutex_),
      has_cb_(true) {
  thread_.Start(&ProposeQueue::StartWorking, this);
}

ProposeQueue::~ProposeQueue() {
  if (exit_ != true) {
    exit_ = true;
    thread_.Join();
  }
}

void ProposeQueue::Put(const ProposeHandler& f, const ProposeCompleteCallback& cb) {
  MutexLock lock(&mutex_);
  propose_queue_.push(f);
  cb_queue_.push(cb);
  cond_.Signal();
}

void* ProposeQueue::StartWorking(void* data) {
  ProposeQueue* temp = reinterpret_cast<ProposeQueue*>(data);
  temp->Propose();
  return nullptr;
}

void ProposeQueue::Propose() {
  while (!exit_) {
    MutexLock lock(&mutex_);
    while (propose_queue_.empty() || !has_cb_) {
      cond_.Wait();
    }
    has_cb_ = false;
    config_->GetLoop()->QueueInLoop(propose_queue_.front());
    propose_queue_.pop();
  }
}

void ProposeQueue::ProposeComplete(MachineContext* context, const Status& s) {
  ProposeCompleteCallback cb;
  {
    MutexLock lock(&mutex_);
    assert(!has_cb_);
    assert(!cb_queue_.empty());
    cb = cb_queue_.front();
    cb_queue_.pop();
    has_cb_ = true;
    cond_.Signal();
  }
  cb(context, s);
}

}  // namespace skywalker
