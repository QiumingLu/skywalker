// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/propose_queue.h"

#include <utility>

#include "skywalker/logging.h"

namespace skywalker {

ProposeQueue::ProposeQueue(size_t capacity)
    : capacity_(capacity), mutex_(), last_finished_(true) {}

ProposeQueue::~ProposeQueue() {}

bool ProposeQueue::CheckCapacity() const {
  if (capacity_ != 0 && propose_queue_.size() > capacity_) {
    LOG_WARN("Too many proposals are waiting to be proposed!");
    return false;
  }
  return true;
}

bool ProposeQueue::Put(ProposeHandler f, ProposeCompleteCallback cb) {
  std::lock_guard<std::mutex> lock(mutex_);
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

void ProposeQueue::ProposeComplete(uint64_t instance_id, const Status& s,
                                   void* context) {
  std::lock_guard<std::mutex> lock(mutex_);
  assert(!last_finished_);
  assert(!cb_queue_.empty());
  ProposeCompleteCallback cb = cb_queue_.front();
  cb_queue_.pop();
  callback_loop_->QueueInLoop(
      [cb, context, s, instance_id]() { cb(instance_id, s, context); });

  if (!propose_queue_.empty()) {
    io_loop_->QueueInLoop(propose_queue_.front());
    propose_queue_.pop();
  } else {
    last_finished_ = true;
  }
}

}  // namespace skywalker
