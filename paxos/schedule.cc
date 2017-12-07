// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/schedule.h"

#include <assert.h>

namespace skywalker {

ThreadPool::ThreadPool() : started_(false), io_next_(0), callback_next_(0) {}

void ThreadPool::Start(uint32_t io_thread_size, uint32_t callback_thread_size) {
  assert(!started_);
  started_ = true;

  io_loops_.reserve(io_thread_size);
  callback_loops_.reserve(callback_thread_size);
  io_threads_.reserve(io_thread_size);
  callback_threads_.reserve(callback_thread_size);

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

RunLoop* ThreadPool::GetNextIOLoop() {
  assert(started_);
  if (io_next_ == io_loops_.size()) {
    io_next_ = 0;
  }
  return io_loops_[io_next_++];
}

RunLoop* ThreadPool::GetNextCallbackLoop() {
  assert(started_);
  if (callback_next_ == callback_loops_.size()) {
    callback_next_ = 0;
  }
  return callback_loops_[callback_next_++];
}

Schedule::Schedule()
    : clean_loop_(nullptr), learn_loop_(nullptr), master_loop_(nullptr) {
  clean_loop_ = clean_thread_.Loop();
  learn_loop_ = learn_thread_.Loop();
}

Schedule::~Schedule() {}

RunLoop* Schedule::CleanLoop() const { return clean_loop_; }

RunLoop* Schedule::LearnLoop() const { return learn_loop_; }

RunLoop* Schedule::MasterLoop() {
  if (master_loop_ == nullptr) {
    master_loop_ = master_thread_.Loop();
  }
  return master_loop_;
}

}  // namespace skywalker
