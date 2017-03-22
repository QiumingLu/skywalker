// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/timerlist.h"

#include <sys/time.h>
#include <time.h>

#include "util/runloop.h"

namespace skywalker {

uint64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec)*1000000 + tv.tv_usec;
}

TimerList::TimerList(RunLoop* loop) 
    : loop_(loop) {
}

TimerList::~TimerList() {
  for (auto& t : timers_) {
    delete t;
  }
}

Timer* TimerList::RunAt(uint64_t micros_value,
                        const TimerProcCallback& cb) {
  Timer* timer(new Timer(micros_value, 0, cb));
  InsertInLoop(timer);
  return timer;
}

Timer* TimerList::RunAt(uint64_t micros_value,
                        TimerProcCallback&& cb) {
  Timer* timer(new Timer(micros_value, 0, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

Timer* TimerList::RunAfter(uint64_t micros_delay,
                           const TimerProcCallback& cb) {
  Timer* timer(new Timer(NowMicros() + micros_delay, 0, cb));
  InsertInLoop(timer);
  return timer;
}

Timer* TimerList::RunAfter(uint64_t micros_delay,
                           TimerProcCallback&& cb) {
  Timer* timer(new Timer(NowMicros() + micros_delay, 0, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

Timer* TimerList::RunEvery(uint64_t micros_interval,
                           const TimerProcCallback& cb) {
  Timer* timer(new Timer(NowMicros() + micros_interval, micros_interval, cb));
  InsertInLoop(timer);
  return timer;
}

Timer* TimerList::RunEvery(uint64_t micros_interval,
                           TimerProcCallback&& cb) {
  Timer* timer(
      new Timer(NowMicros() + micros_interval, micros_interval, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

void TimerList::Remove(Timer* timer) {
  loop_->QueueInLoop([timer, this]() {
    std::set<Timer*>::iterator it = timers_.find(timer);
    if (it != timers_.end()) {
      timers_.erase(it);
      delete timer;
    }
  });
}

void TimerList::InsertInLoop(Timer* timer) {
  loop_->QueueInLoop([timer, this]() {
    timers_.insert(timer);
  });
}
uint64_t TimerList::TimeoutMicros() const {
  if (timers_.empty()) {
    return -1;
  }
  std::set<Timer*>::iterator it = timers_.begin();
  if ((*it)->micros_value < NowMicros()) {
    return 0;
  } else {
    return ((*it)->micros_value - NowMicros());
  }
}

void TimerList::RunTimerProcs() {
  if (timers_.empty()) {
    return;
  }

  uint64_t micros_now = NowMicros();
  std::set<Timer*>::iterator it;
  while (true) {
    it = timers_.begin();
    if (it != timers_.end() && (*it)->micros_value <= micros_now) {
      Timer* timer = *it;
      timers_.erase(it);
      timer->timerproc_cb();
      if (timer->repeat) {
        timer->micros_value += timer->micros_interval;
        timers_.insert(timer);
      } else {
        delete timer;
      }
    } else {
      break;
    }
  }
}

}  // namespace skywalker
