// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_TIMERLIST_H_
#define SKYWALKER_UTIL_TIMERLIST_H_

#include <stdint.h>
#include <set>
#include <utility>

#include "util/callback.h"

namespace skywalker {

class RunLoop;

class Timer;

typedef std::pair<uint64_t, Timer*> TimerId;

class TimerList {
 public:
  explicit TimerList(RunLoop* loop);
  ~TimerList();

  TimerId RunAt(uint64_t micros_value, const TimerProcCallback& cb);
  TimerId RunAt(uint64_t micros_value, TimerProcCallback&& cb);

  TimerId RunAfter(uint64_t micros_delay, const TimerProcCallback& cb);
  TimerId RunAfter(uint64_t micros_delay, TimerProcCallback&& cb);

  TimerId RunEvery(uint64_t micros_interval, const TimerProcCallback& cb);
  TimerId RunEvery(uint64_t micros_interval, TimerProcCallback&& cb);

  void Remove(TimerId timer);

  uint64_t TimeoutMicros() const;
  void RunTimerProcs();

 private:
  void InsertInLoop(TimerId timer);

  RunLoop* loop_;
  std::set<Timer*> timer_ptrs_;
  std::set<TimerId> timers_;

  // No copying allowed
  TimerList(const TimerList&);
  void operator=(const TimerList&);
};

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_TIMERLIST_H_
