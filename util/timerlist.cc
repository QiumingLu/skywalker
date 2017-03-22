// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/timerlist.h"

#include <sys/time.h>
#include <time.h>

#include "util/runloop.h"

namespace skywalker {

class Timer {
 private:
  friend class TimerList;

  Timer(uint64_t value, uint64_t interval, const TimerProcCallback& cb)
      : micros_value(value),
        micros_interval(interval),
        timerproc_cb(cb),
        repeat(false) {
    if (micros_interval > 0) {
      repeat = true;
    }
  }

  Timer(uint64_t value, uint64_t interval, TimerProcCallback&& cb)
      : micros_value(value),
        micros_interval(interval),
        timerproc_cb(std::move(cb)),
        repeat(false) {
    if (micros_interval > 0) {
      repeat = true;
    }
  }

  ~Timer() {
  }

  uint64_t micros_value;
  uint64_t micros_interval;
  TimerProcCallback timerproc_cb;
  bool repeat;
};

uint64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec)*1000000 + tv.tv_usec;
}

TimerList::TimerList(RunLoop* loop) 
    : loop_(loop) {
}

TimerList::~TimerList() {
  for (auto& t : timer_ptrs_) {
    delete t;
  }
}

TimerId TimerList::RunAt(uint64_t micros_value,
                         const TimerProcCallback& cb) {
  TimerId timer(micros_value, new Timer(micros_value, 0, cb));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunAt(uint64_t micros_value,
                         TimerProcCallback&& cb) {
  TimerId timer(micros_value, new Timer(micros_value, 0, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunAfter(uint64_t micros_delay,
                            const TimerProcCallback& cb) {
  uint64_t micros_value = NowMicros() + micros_delay;
  TimerId timer(micros_value, new Timer(micros_value, 0, cb));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunAfter(uint64_t micros_delay,
                            TimerProcCallback&& cb) {
  uint64_t micros_value = NowMicros() + micros_delay;
  TimerId timer(micros_value, new Timer(micros_value, 0, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunEvery(uint64_t micros_interval,
                            const TimerProcCallback& cb) {
  uint64_t micros_value = NowMicros() + micros_interval;
  TimerId timer(micros_value, new Timer(micros_value, micros_interval, cb));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunEvery(uint64_t micros_interval,
                            TimerProcCallback&& cb) {
  uint64_t micros_value = NowMicros() + micros_interval;
  TimerId timer(micros_value, 
                new Timer(micros_value, micros_interval, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

void TimerList::Remove(TimerId timer) {
  loop_->RunInLoop([timer, this]() {
    if (timer_ptrs_.find(timer.second) != timer_ptrs_.end()) {
      TimerId t(timer.second->micros_value, timer.second);
      std::set<TimerId>::iterator it = timers_.find(t);
      if (it != timers_.end()) {
        delete it->second;
        timers_.erase(it);
        timer_ptrs_.erase(it->second);
      }
    }
  });
}

void TimerList::InsertInLoop(TimerId timer) {
  loop_->RunInLoop([timer, this]() {
    timers_.insert(timer);
    timer_ptrs_.insert(timer.second);
  });
}

uint64_t TimerList::TimeoutMicros() const {
  loop_->AssertInMyLoop();
  if (timers_.empty()) {
    return -1;
  }
  std::set<TimerId>::iterator it = timers_.begin();
  if (it->first < NowMicros()) {
    return 0;
  } else {
    return (it->first - NowMicros());
  }
}

void TimerList::RunTimerProcs() {
  loop_->AssertInMyLoop();
  if (timers_.empty()) {
    return;
  }

  uint64_t micros_now = NowMicros();
  std::set<TimerId>::iterator it;
  while (true) {
    it = timers_.begin();
    if (it != timers_.end() && it->first <= micros_now) {
      Timer* timer = it->second;
      timers_.erase(it);
      timer->timerproc_cb();
      if (timer->repeat) {
        timer->micros_value += timer->micros_interval;
        timers_.insert(std::make_pair(timer->micros_value, timer));
      } else {
        delete timer;
        timer_ptrs_.erase(timer);
      }
    } else {
      break;
    }
  }
}

}  // namespace skywalker
