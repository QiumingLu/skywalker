// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/propose_queue.h"
#include "skywalker/logging.h"

namespace skywalker {

ProposeQueue::ProposeQueue(Config* config)
    : config_(config),
      exit_(false),
      thread_(),
      mutex_(),
      cond_(&mutex_),
      has_cb_(true),
      capacity_(0) {
  thread_.Start(&ProposeQueue::StartWorking, this);
}

ProposeQueue::~ProposeQueue() {
  if (exit_ != true) {
    exit_ = true;
    thread_.Join();
  }
}

bool ProposeQueue::Put(const ProposeHandler& f,
                       const ProposeCompleteCallback& cb) {
  MutexLock lock(&mutex_);
  if (capacity_ != 0 && propose_queue_.size() > capacity_) {
    SWLog(WARN, "Too many proposals are waiting to be proposed!");
    return false; 
  }
  propose_queue_.push(f);
  cb_queue_.push(cb);
  cond_.Signal();
  return true;
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

void ProposeQueue::ProposeComplete(
    MachineContext* context, const Status& s, uint64_t instance_id) {
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
  cb(context, s, instance_id);
}

}  // namespace skywalker
