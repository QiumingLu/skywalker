#ifndef SKYWALKER_UTIL_TIMERLIST_H_
#define SKYWALKER_UTIL_TIMERLIST_H_

#include <stdint.h>
#include <set>
#include <vector>
#include <utility>

#include "util/callback.h"

namespace skywalker {

extern uint64_t NowMicros();

class TimerList;

class Timer {
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

typedef std::pair<uint64_t, Timer*> TimerId;

class TimerList {
 public:
  explicit TimerList();
  ~TimerList();

  TimerId RunAt(uint64_t micros_value, const TimerProcCallback& cb);
  TimerId RunAt(uint64_t micros_value, TimerProcCallback&& cb);

  TimerId RunAfter(uint64_t micros_delay, const TimerProcCallback& cb);
  TimerId RunAfter(uint64_t micros_delay, TimerProcCallback&& cb);

  TimerId RunEvery(uint64_t micros_interval, const TimerProcCallback& cb);
  TimerId RunEvery(uint64_t micros_interval, TimerProcCallback&& cb);

  void Remove(const TimerId& id);

  uint64_t TimeoutMicros() const;
  void RunTimerProcs();

 private:
  std::set<TimerId> timers_;

  // No copying allowed
  TimerList(const TimerList&);
  void operator=(const TimerList&);
};

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_TIMERLIST_H_
