// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/propose_queue.h"

#include <utility>

#include "util/mutexlock.h"
#include "skywalker/logging.h"

namespace skywalker {

ProposeQueue::ProposeQueue(size_t capacity)
    : capacity_(capacity),
      mutex_(),
      last_finished_(true) {
}

ProposeQueue::~ProposeQueue() {
}

bool ProposeQueue::CheckCapacity() const {
  if (capacity_ != 0 && propose_queue_.size() > capacity_) {
    LOG_WARN("Too many proposals are waiting to be proposed!");
    return false;
  }
  return true;
}

bool ProposeQueue::Put(const ProposeHandler& f,
                       const ProposeCompleteCallback& cb) {
  MutexLock lock(&mutex_);
  if (last_finished_) {
    last_finished_ = false;
    io_loop_->QueueInLoop(f);
  } else {
    if (!CheckCapacity()) {
      return false;
    }
    propose_queue_.push(f);
  }
  cb_queue_.push(cb);
  return true;
}

bool ProposeQueue::Put(ProposeHandler&& f,
                       const ProposeCompleteCallback& cb) {
  MutexLock lock(&mutex_);
  if (last_finished_) {
    last_finished_ = false;
    io_loop_->QueueInLoop(std::move(f));
  } else {
    if (!CheckCapacity()) {
      return false;
    }
    propose_queue_.push(std::move(f));
  }
  cb_queue_.push(cb);
  return true;
}

bool ProposeQueue::Put(const ProposeHandler& f,
                       ProposeCompleteCallback&& cb) {
  MutexLock lock(&mutex_);
  if (last_finished_) {
    last_finished_ = false;
    io_loop_->QueueInLoop(f);
  } else {
    if (!CheckCapacity()) {
      return false;
    }
    propose_queue_.push(f);
  }
  cb_queue_.push(std::move(cb));
  return true;
}

bool ProposeQueue::Put(ProposeHandler&& f,
                       ProposeCompleteCallback&& cb) {
  MutexLock lock(&mutex_);
  if (last_finished_) {
    last_finished_ = false;
    io_loop_->QueueInLoop(std::move(f));
  } else {
    if (!CheckCapacity()) {
      return false;
    }
    propose_queue_.push(std::move(f));
  }
  cb_queue_.push(std::move(cb));
  return true;
}

void ProposeQueue::ProposeComplete(
    MachineContext* context, const Status& s, uint64_t instance_id) {
  MutexLock lock(&mutex_);
  assert(!last_finished_);
  assert(!cb_queue_.empty());
  ProposeCompleteCallback cb;
  cb = cb_queue_.front();
  cb_queue_.pop();
  callback_loop_->QueueInLoop([cb, context, s, instance_id]() {
    cb(context, s, instance_id);
  });

  if (!propose_queue_.empty()) {
    io_loop_->QueueInLoop(propose_queue_.front());
    propose_queue_.pop();
  } else {
    last_finished_ = true;
  }
}

}  // namespace skywalker
