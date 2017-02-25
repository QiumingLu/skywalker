// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/runloop.h"

#include <algorithm>
#include <utility>

#include "paxos/instance.h"
#include "util/mutexlock.h"

namespace skywalker {

void* RunLoop::StartRunLoop(void* data) {
  RunLoop* loop = reinterpret_cast<RunLoop*>(data);
  loop->ThreadFunc();
  return nullptr;
}

RunLoop::RunLoop()
    : exit_(false),
      instance_(nullptr),
      thread_(),
      mutex_(),
      cond_(&mutex_) {
}

RunLoop::~RunLoop() {
  if (exit_ != true) {
    exit_ = true;
    thread_.Join();
  }
}

void RunLoop::Loop() {
  assert(!thread_.Started());
  thread_.Start(&RunLoop::StartRunLoop, this);
}

void RunLoop::Exit() {
  exit_ = true;
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

void RunLoop::ThreadFunc() {
  exit_ = false;
  while (!exit_) {
    uint64_t t = timers_.TimeoutMicros();
    uint64_t timeout = std::min(t, static_cast<uint64_t>(5000 * 1000));
    std::vector<Func> funcs;
    {
      MutexLock lock(&mutex_);
      while (funcs_.empty()) {
        cond_.Wait(timeout);
        break;
      }
      funcs.swap(funcs_);
    }
    for (auto f : funcs) {
      f();
    }
    timers_.RunTimerProcs();
  }
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

void RunLoop::Remove(const TimerId& t) {
  timers_.Remove(t);
}

}  // namespace skywalker
