// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_RUNLOOP_H_
#define SKYWALKER_UTIL_RUNLOOP_H_

#include <stdint.h>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "util/timerlist.h"

namespace skywalker {

class RunLoop {
 public:
  typedef std::function<void()> Func;

  RunLoop();

  void Loop();
  void Exit();

  bool IsInMyLoop() const;
  void AssertInMyLoop();

  void RunInLoop(const Func& func);
  void RunInLoop(Func&& func);
  void QueueInLoop(const Func& func);
  void QueueInLoop(Func&& func);

  TimerId RunAt(uint64_t ms_value, const TimerProcCallback& cb);
  TimerId RunAfter(uint64_t ms_delay, const TimerProcCallback& cb);
  TimerId RunEvery(uint64_t ms_interval, const TimerProcCallback& cb);

  TimerId RunAt(uint64_t ms_value, TimerProcCallback&& cb);
  TimerId RunAfter(uint64_t ms_delay, TimerProcCallback&& cb);
  TimerId RunEvery(uint64_t ms_interval, TimerProcCallback&& cb);

  void Remove(TimerId t);

 private:
  bool exit_;
  const std::thread::id tid_;

  std::mutex mutex_;
  std::condition_variable cond_;
  std::vector<Func> funcs_;
  TimerList timers_;

  // No copying allowed
  RunLoop(const RunLoop&);
  void operator=(const RunLoop&);
};

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_RUNLOOP_H_
