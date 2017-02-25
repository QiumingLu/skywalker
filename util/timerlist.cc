// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/timerlist.h"
#include <sys/time.h>
#include <time.h>

namespace skywalker {

uint64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec)*1000000 + tv.tv_usec;
}

TimerList::TimerList() {
}

TimerList::~TimerList() {
  for (auto t : timers_) {
    delete t.second;
  }
}

TimerId TimerList::RunAt(uint64_t micros_value,
                         const TimerProcCallback& cb) {
  TimerId id(micros_value, new Timer(micros_value, 0, cb));
  timers_.insert(id);
  return id;
}

TimerId TimerList::RunAt(uint64_t micros_value,
                         TimerProcCallback&& cb) {
  TimerId id(micros_value, new Timer(micros_value, 0, std::move(cb)));
  timers_.insert(id);
  return id;
}

TimerId TimerList::RunAfter(uint64_t micros_delay,
                            const TimerProcCallback& cb) {
  uint64_t micros_value = NowMicros() + micros_delay;
  TimerId id(micros_value, new Timer(micros_value, 0, cb));
  timers_.insert(id);
  return id;
}

TimerId TimerList::RunAfter(uint64_t micros_delay,
                            TimerProcCallback&& cb) {
  uint64_t micros_value = NowMicros() + micros_delay;
  TimerId id(micros_value, new Timer(micros_value, 0, std::move(cb)));
  timers_.insert(id);
  return id;
}

TimerId TimerList::RunEvery(uint64_t micros_interval,
                            const TimerProcCallback& cb) {
  uint64_t micros_value = NowMicros() + micros_interval;
  TimerId id(micros_value, new Timer(micros_value, micros_interval, cb));
  timers_.insert(id);
  return id;
}

TimerId TimerList::RunEvery(uint64_t micros_interval,
                            TimerProcCallback&& cb) {
  uint64_t micros_value = NowMicros() + micros_interval;
  TimerId id(micros_value,
             new Timer(micros_value, micros_interval, std::move(cb)));
  timers_.insert(id);
  return id;
}

void TimerList::Remove(const TimerId& id) {
  std::set<TimerId>::iterator it = timers_.find(id);
  if (it != timers_.end()) {
    delete it->second;
    timers_.erase(it);
  }
}

uint64_t TimerList::TimeoutMicros() const {
  if (timers_.empty()) {
    return 5000000;
  }
  if (timers_.begin()->first < NowMicros()) {
    return 0;
  } else {
    return timers_.begin()->first - NowMicros();
  }
}

void TimerList::RunTimerProcs() {
  if (timers_.empty()) {
    return;
  }

  uint64_t micros_now = NowMicros();
  std::set<TimerId>::iterator it;
  while (true) {
    it = timers_.begin();
    if (it != timers_.end() && it->first <= micros_now) {
      uint64_t micros_value = it->first;
      Timer* timer = it->second;
      timers_.erase(it);
      timer->timerproc_cb();
      if (timer->repeat) {
        micros_value += timer->micros_interval;
        timers_.insert(TimerId(micros_value, timer));
      } else {
        delete timer;
      }
    } else {
      break;
    }
  }
}

}  // namespace skywalker
