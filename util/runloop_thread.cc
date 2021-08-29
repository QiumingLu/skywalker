// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/runloop_thread.h"

#include <assert.h>

namespace skywalker {

RunLoopThread::RunLoopThread() : loop_(nullptr) {}

RunLoopThread::~RunLoopThread() {
  if (loop_ != nullptr) {
    loop_->Exit();
  }
  if (thread_) {
    thread_->join();
  }
}

RunLoop* RunLoopThread::Loop() {
  assert(!thread_);
  thread_.reset(new std::thread([this]() { ThreadFunc(); }));
  std::unique_lock<std::mutex> lock(mu_);
  while (loop_ == nullptr) {
    cond_.wait(lock);
  }
  return loop_;
}

void RunLoopThread::ThreadFunc() {
  RunLoop loop;
  {
    std::unique_lock<std::mutex> lock(mu_);
    loop_ = &loop;
    cond_.notify_one();
  }
  loop_->Loop();
  loop_ = nullptr;
}

}  // namespace skywalker
