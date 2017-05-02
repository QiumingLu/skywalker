// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/runloop.h"

#include <assert.h>

#include <algorithm>
#include <utility>

#include "util/mutexlock.h"
#include "util/thread.h"
#include "skywalker/logging.h"

namespace skywalker {

RunLoop::RunLoop()
    : exit_(false),
      tid_(CurrentThread::Tid()),
      mutex_(),
      cond_(&mutex_),
      timers_(this) {
}

void RunLoop::Loop() {
  AssertInMyLoop();
  exit_ = false;
  while (!exit_) {
    uint64_t t = timers_.TimeoutMicros();
    uint64_t timeout = std::min(t, static_cast<uint64_t>(5000 * 1000));
    std::vector<Func> funcs;
    {
      MutexLock lock(&mutex_);
      if (funcs_.empty()) {
        cond_.Wait(timeout);
      }
      funcs.swap(funcs_);
    }
    for (auto& f : funcs) {
      f();
    }
    timers_.RunTimerProcs();
  }
}

void RunLoop::Exit() {
  this->QueueInLoop([this]() {
    exit_ = true;
  });
}

bool RunLoop::IsInMyLoop() const {
  return tid_ == CurrentThread::Tid();
}

void RunLoop::AssertInMyLoop() {
  if (!IsInMyLoop()) {
    LOG_FATAL("runloop tid=%llu, but current thread tid=%llu",
              static_cast<unsigned long long>(tid_),
              static_cast<unsigned long long>(CurrentThread::Tid()));
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
  MutexLock lock(&mutex_);
  funcs_.push_back(func);
  cond_.Signal();
}

void RunLoop::QueueInLoop(Func&& func) {
  MutexLock lock(&mutex_);
  funcs_.push_back(std::move(func));
  cond_.Signal();
}

TimerId RunLoop::RunAt(uint64_t micros_value,
                       const TimerProcCallback& cb) {
  return timers_.RunAt(micros_value, cb);
}

TimerId RunLoop::RunAfter(uint64_t micros_delay,
                         const TimerProcCallback& cb) {
  return timers_.RunAfter(micros_delay, cb);
}

TimerId RunLoop::RunEvery(uint64_t micros_interval,
                         const TimerProcCallback& cb) {
  return timers_.RunEvery(micros_interval, cb);
}

TimerId RunLoop::RunAt(uint64_t micros_value,
                      TimerProcCallback&& cb) {
  return timers_.RunAt(micros_value, std::move(cb));
}

TimerId RunLoop::RunAfter(uint64_t micros_delay,
                         TimerProcCallback&& cb) {
  return timers_.RunAfter(micros_delay, std::move(cb));
}

TimerId RunLoop::RunEvery(uint64_t micros_interval,
                         TimerProcCallback&& cb) {
  return timers_.RunEvery(micros_interval, std::move(cb));
}

void RunLoop::Remove(TimerId t) {
  timers_.Remove(t);
}

}  // namespace skywalker
