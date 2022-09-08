// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/threadpool.h"

#include <assert.h>

namespace skywalker {

ThreadPool::ThreadPool()
    : started_(false), io_next_(0), callback_next_(0),
      learn_next_(0), clean_next_(0), master_next_(0) {}

void ThreadPool::Start(uint32_t io_thread_size, uint32_t callback_thread_size,
                       uint32_t learn_thread_size, uint32_t clean_thread_size,
                       uint32_t master_thread_size) {
  assert(!started_);
  started_ = true;

  io_loops_.reserve(io_thread_size);
  callback_loops_.reserve(callback_thread_size);
  learn_loops_.reserve(learn_thread_size);
  clean_loops_.reserve(clean_thread_size);
  master_loops_.reserve(master_thread_size);

  io_threads_.reserve(io_thread_size);
  callback_threads_.reserve(callback_thread_size);
  learn_threads_.reserve(learn_thread_size);
  clean_threads_.reserve(clean_thread_size);
  master_threads_.reserve(master_thread_size);

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

  for (uint32_t i = 0; i < learn_thread_size; ++i) {
    RunLoopThread* thread = new RunLoopThread();
    learn_loops_.push_back(thread->Loop());
    learn_threads_.push_back(std::unique_ptr<RunLoopThread>(thread));
  }

  for (uint32_t i = 0; i < clean_thread_size; ++i) {
    RunLoopThread* thread = new RunLoopThread();
    clean_loops_.push_back(thread->Loop());
    clean_threads_.push_back(std::unique_ptr<RunLoopThread>(thread));
  }

  for (uint32_t i = 0; i < master_thread_size; ++i) {
    RunLoopThread* thread = new RunLoopThread();
    master_loops_.push_back(thread->Loop());
    master_threads_.push_back(std::unique_ptr<RunLoopThread>(thread));
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


RunLoop* ThreadPool::GetNextLearnLoop() {
  assert(started_);
  if (learn_next_ == learn_loops_.size()) {
    learn_next_ = 0;
  }
  return learn_loops_[learn_next_++];
}

RunLoop* ThreadPool::GetNextCleanLoop() {
  assert(started_);
  if (clean_next_ == clean_loops_.size()) {
    clean_next_ = 0;
  }
  return clean_loops_[clean_next_++]; 
}

RunLoop* ThreadPool::GetNextMasterLoop() {
  assert(started_);
  if (master_loops_.empty()) {
    return nullptr;
  }
  if (master_next_ == master_loops_.size()) {
    master_next_ = 0;
  }
  return master_loops_[master_next_++];
}

}  // namespace skywalker
