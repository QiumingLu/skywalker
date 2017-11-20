// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/schedule.h"

#include <assert.h>

namespace skywalker {

Schedule::Schedule()
    : started_(false),
      io_next_(0),
      callback_next_(0),
      clean_loop_(nullptr),
      learn_loop_(nullptr),
      master_loop_(nullptr) {}

Schedule::~Schedule() {}

void Schedule::Start(uint32_t io_thread_size, uint32_t callback_thread_size,
                     bool use_master) {
  assert(!started_);
  started_ = true;
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
  for (uint32_t i = 0; i < callback_thread_size; ++i) {
    RunLoopThread* thread = new RunLoopThread();
    callback_loops_.push_back(thread->Loop());
    callback_threads_.push_back(std::unique_ptr<RunLoopThread>(thread));
  }
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
  if (io_next_ == io_loops_.size()) {
    io_next_ = 0;
  }
  return io_loops_[io_next_++];
}

RunLoop* Schedule::GetNextCallbackLoop() {
  assert(started_);
  if (callback_next_ == callback_loops_.size()) {
    callback_next_ = 0;
  }
  return callback_loops_[callback_next_++];
}

}  // namespace skywalker
