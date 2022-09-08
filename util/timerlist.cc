// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/timerlist.h"
#include "util/runloop.h"
#include "util/timeops.h"

namespace skywalker {

class Timer {
 private:
  friend class TimerList;

  Timer(uint64_t value, uint64_t interval, const TimerProcCallback& cb)
      : ms_value(value), ms_interval(interval), timerproc_cb(cb) {}

  Timer(uint64_t value, uint64_t interval, TimerProcCallback&& cb)
      : ms_value(value),
        ms_interval(interval),
        timerproc_cb(std::move(cb)) {}

  ~Timer() {}

  uint64_t ms_value;
  uint64_t ms_interval;
  TimerProcCallback timerproc_cb;
};

TimerList::TimerList(RunLoop* loop)
    : last_time_out_(NowMillis()), loop_(loop) {}

TimerList::~TimerList() {
  for (auto& t : timer_ptrs_) {
    delete t;
  }
}

TimerId TimerList::RunAt(uint64_t ms_value, const TimerProcCallback& cb) {
  TimerId timer(ms_value, new Timer(ms_value, 0, cb));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunAt(uint64_t ms_value, TimerProcCallback&& cb) {
  TimerId timer(ms_value, new Timer(ms_value, 0, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunAfter(uint64_t ms_delay,
                            const TimerProcCallback& cb) {
  uint64_t ms_value = NowMillis() + ms_delay;
  TimerId timer(ms_value, new Timer(ms_value, 0, cb));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunAfter(uint64_t ms_delay, TimerProcCallback&& cb) {
  uint64_t ms_value = NowMillis() + ms_delay;
  TimerId timer(ms_value, new Timer(ms_value, 0, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunEvery(uint64_t ms_interval,
                            const TimerProcCallback& cb) {
  uint64_t ms_value = NowMillis() + ms_interval;
  TimerId timer(ms_value, new Timer(ms_value, ms_interval, cb));
  InsertInLoop(timer);
  return timer;
}

TimerId TimerList::RunEvery(uint64_t ms_interval, TimerProcCallback&& cb) {
  uint64_t ms_value = NowMillis() + ms_interval;
  TimerId timer(ms_value,
                new Timer(ms_value, ms_interval, std::move(cb)));
  InsertInLoop(timer);
  return timer;
}

void TimerList::Remove(TimerId timer) {
  loop_->RunInLoop([timer, this]() mutable {
    if (timer_ptrs_.find(timer.second) != timer_ptrs_.end()) {
      timer_ptrs_.erase(timer.second);
      timer.first = timer.second->ms_value;
      delete timer.second;
      timers_.erase(timer);
    }
  });
}

void TimerList::InsertInLoop(TimerId timer) {
  loop_->RunInLoop([timer, this]() {
    timers_.insert(timer);
    timer_ptrs_.insert(timer.second);
  });
}

uint64_t TimerList::TimeoutMs() const {
  loop_->AssertInMyLoop();
  if (timers_.empty()) {
    return -1;
  }
  std::set<TimerId>::const_iterator it = timers_.begin();
  uint64_t now = NowMillis();
  if (now < last_time_out_) {
    return 0;
  }
  if (it->first <= now) {
    return 0;
  } else {
    return (it->first - now);
  }
}

void TimerList::RunTimerProcs() {
  loop_->AssertInMyLoop();
  if (timers_.empty()) {
    return;
  }

  uint64_t ms_now = NowMillis();

  if (ms_now < last_time_out_) {
    uint64_t diff = last_time_out_ - ms_now;
    std::set<TimerId> timers;
    for (auto& it : timers_) {
      uint64_t value = it.first > diff ? it.first - diff : 0;
      it.second->ms_value = value;
      timers.insert(TimerId(value, it.second));
    }
    timers_.swap(timers);
  }
  last_time_out_ = ms_now;

  std::set<TimerId>::iterator it;
  while (true) {
    it = timers_.begin();
    if (it != timers_.end() && it->first <= ms_now) {
      Timer* t = it->second;
      timers_.erase(it);
      TimerProcCallback cb = t->timerproc_cb;
      if (t->ms_interval > 0) {
        t->ms_value = ms_now + t->ms_interval;
        TimerId timer(t->ms_value, t);
        timers_.insert(timer);
      } else {
        delete t;
        timer_ptrs_.erase(t);
      }
      cb();
    } else {
      break;
    }
  }
}

}  // namespace skywalker
