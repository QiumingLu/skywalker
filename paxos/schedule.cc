// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/schedule.h"

#include <assert.h>

namespace skywalker {

Schedule::Schedule()
    : started_(false),
      next_(0),
      callback_loop_(nullptr),
      learn_loop_(nullptr),
      master_loop_(nullptr) {}

Schedule::~Schedule() {}

void Schedule::Start(uint32_t io_thread_size, bool use_master) {
  assert(!started_);
  started_ = true;
  callback_loop_ = callback_thread_.Loop();
  clean_loop_ = clean_thread_.Loop();
  learn_loop_ = learn_thread_.Loop();
  if (use_master) {
    master_loop_ = master_thread_.Loop();
  }
  for (uint32_t i = 0; i < io_thread_size; ++i) {
    RunLoopThread* thread = new RunLoopThread();
    io_loops_.push_back(thread->Loop());
    io_threads_.push_back(std::unique_ptr<RunLoopThread>(thread));
  }
}

RunLoop* Schedule::CallbackLoop() const {
  assert(started_);
  return callback_loop_;
}

RunLoop* Schedule::CleanLoop() const {
  assert(started_);
  return clean_loop_;
}

RunLoop* Schedule::LearnLoop() const {
  assert(started_);
  return learn_loop_;
}

RunLoop* Schedule::MasterLoop() const {
  assert(started_);
  return master_loop_;
}

RunLoop* Schedule::GetNextIOLoop() {
  assert(started_);
  if (next_ == io_loops_.size()) {
    next_ = 0;
  }
  return io_loops_[next_++];
}

}  // namespace skywalker
