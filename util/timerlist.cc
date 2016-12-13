#include "util/timerlist.h"
#include <sys/time.h>
#include <time.h>

namespace skywalker {

uint64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec)*1000000 + tv.tv_usec;
}

struct TimerList::Timer {
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

 private:
  friend class TimerList;

  uint64_t micros_value;
  uint64_t micros_interval;
  TimerProcCallback timerproc_cb;
  bool repeat;
};


TimerList::TimerList() {
}

TimerList::~TimerList() {
  for (auto t : timers_) {
    delete t.second;
  }
}

TimerList::Timer* TimerList::RunAt(uint64_t micros_value,
                                   const TimerProcCallback& cb) {
   Timer* timer = new Timer(micros_value, 0, cb);
   timers_.insert(Entry(timer->micros_value, timer));
   return timer;
}

TimerList::Timer* TimerList::RunAt(uint64_t micros_value,
                                   TimerProcCallback&& cb) {
   Timer* timer = new Timer(micros_value, 0, std::move(cb));
   timers_.insert(Entry(timer->micros_value, timer));
   return timer;
}

TimerList::Timer* TimerList::RunAfter(uint64_t micros_delay,
                                      const TimerProcCallback& cb) {
   uint64_t micros_value = NowMicros() + micros_delay;
   Timer* timer = new Timer(micros_value, 0, cb);
   timers_.insert(Entry(timer->micros_value, timer));
   return timer;
}

TimerList::Timer* TimerList::RunAfter(uint64_t micros_delay,
                                      TimerProcCallback&& cb) {
   uint64_t micros_value = NowMicros() + micros_delay;
   Timer* timer = new Timer(micros_value, 0, std::move(cb));
   timers_.insert(Entry(timer->micros_value, timer));
   return timer;
}

TimerList::Timer* TimerList::RunEvery(uint64_t micros_interval,
                                      const TimerProcCallback& cb) {
   uint64_t micros_value = NowMicros() + micros_interval;
   Timer* timer = new Timer(micros_value, micros_interval, cb);
   timers_.insert(Entry(timer->micros_value, timer));
   return timer;
}

TimerList::Timer* TimerList::RunEvery(uint64_t micros_interval,
                                      TimerProcCallback&& cb) {
   uint64_t micros_value = NowMicros() + micros_interval;
   Timer* timer = new Timer(micros_value, micros_interval, std::move(cb));
   timers_.insert(Entry(timer->micros_value, timer));
   return timer;
}

void TimerList::Remove(Timer* timer) {
  Entry entry(timer->micros_value, timer);
  std::set<Entry>::iterator it = timers_.find(entry);
  if (it != timers_.end()) {
    delete it->second;
    timers_.erase(it);
  }
}

uint64_t TimerList::TimeoutMicros() const {
  if (timers_.empty()) {
    return 1000000;
  }
  return (timers_.begin()->first - NowMicros());
}

void TimerList::RunTimerProcs() {
  if (timers_.empty()) {
    return;
  }

  uint64_t micros_now = NowMicros();
  std::vector<Timer*> res(ExpiredTimers(micros_now));

  for (size_t i = 0; i < res.size(); ++i) {
    res[i]->timerproc_cb();

    if (res[i]->repeat) {
      res[i]->micros_value = micros_now + res[i]->micros_interval;
      timers_.insert(Entry(res[i]->micros_value, res[i]));
    } else {
      delete res[i];
    }
  }
}

std::vector<TimerList::Timer*> TimerList::ExpiredTimers(uint64_t micros) {
  std::vector<Timer*> res;
  Entry entry(micros, reinterpret_cast<Timer*>(UINTPTR_MAX));
  std::set<Entry>::iterator end = timers_.lower_bound(entry);
  for (std::set<Entry>::iterator it = timers_.begin(); it != end; ++it) {
    res.push_back(it->second);
  }
  timers_.erase(timers_.begin(), end);
  return res;
}

}  // namespace skywalker
