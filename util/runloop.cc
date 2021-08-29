// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/runloop.h"

#include <assert.h>

#include <algorithm>
#include <utility>

#include "skywalker/logging.h"

namespace skywalker {

RunLoop::RunLoop()
    : exit_(false), tid_(std::this_thread::get_id()), timers_(this) {}

void RunLoop::Loop() {
  AssertInMyLoop();
  exit_ = false;
  std::vector<Func> funcs;
  while (!exit_) {
    uint64_t timeout = timers_.TimeoutMicros();
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (funcs_.empty()) {
        cond_.wait_for(lock, std::chrono::microseconds(timeout));
      }
      funcs.swap(funcs_);
    }
    timers_.RunTimerProcs();
    for (auto& f : funcs) {
      f();
    }
    funcs.clear();
  }
}

void RunLoop::Exit() {
  exit_ = true;
  if (!IsInMyLoop()) {
    cond_.notify_one();
  }
}

bool RunLoop::IsInMyLoop() const { return tid_ == std::this_thread::get_id(); }

void RunLoop::AssertInMyLoop() {
  if (!IsInMyLoop()) {
    // LOG_FATAL("runloop tid=%llu, but current thread tid=%llu",
    // static_cast<unsigned long long>(tid_),
    // static_cast<unsigned long long>(std::this_thread::get_id()));
  }
}

void RunLoop::RunInLoop(const Func& func) {
  if (IsInMyLoop()) {
    func();
  } else {
    QueueInLoop(func);
  }
}

void RunLoop::RunInLoop(Func&& func) {
  if (IsInMyLoop()) {
    func();
  } else {
    QueueInLoop(std::move(func));
  }
}

void RunLoop::QueueInLoop(const Func& func) {
  std::unique_lock<std::mutex> lock(mutex_);
  funcs_.push_back(func);
  cond_.notify_one();
}

void RunLoop::QueueInLoop(Func&& func) {
  std::unique_lock<std::mutex> lock(mutex_);
  funcs_.push_back(std::move(func));
  cond_.notify_one();
}

TimerId RunLoop::RunAt(uint64_t micros_value, const TimerProcCallback& cb) {
  return timers_.RunAt(micros_value, cb);
}

TimerId RunLoop::RunAfter(uint64_t micros_delay, const TimerProcCallback& cb) {
  return timers_.RunAfter(micros_delay, cb);
}

TimerId RunLoop::RunEvery(uint64_t micros_interval,
                          const TimerProcCallback& cb) {
  return timers_.RunEvery(micros_interval, cb);
}

TimerId RunLoop::RunAt(uint64_t micros_value, TimerProcCallback&& cb) {
  return timers_.RunAt(micros_value, std::move(cb));
}

TimerId RunLoop::RunAfter(uint64_t micros_delay, TimerProcCallback&& cb) {
  return timers_.RunAfter(micros_delay, std::move(cb));
}

TimerId RunLoop::RunEvery(uint64_t micros_interval, TimerProcCallback&& cb) {
  return timers_.RunEvery(micros_interval, std::move(cb));
}

void RunLoop::Remove(TimerId t) { timers_.Remove(t); }

}  // namespace skywalker
