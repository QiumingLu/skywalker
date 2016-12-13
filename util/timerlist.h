#ifndef SKYWALKER_UTIL_TIMERLIST_H_
#define SKYWALKER_UTIL_TIMERLIST_H_

#include <set>
#include <vector>
#include <utility>

namespace skywalker {

extern uint64_t NowMicros();

class TimerList {
 public:
  struct Timer;
  typedef std::function<void ()> TimerProcCallback;

  explicit TimerList();
  ~TimerList();

  Timer* RunAt(uint64_t micros_value, const TimerProcCallback& cb);
  Timer* RunAt(uint64_t micros_value, TimerProcCallback&& cb);

  Timer* RunAfter(uint64_t micros_delay, const TimerProcCallback& cb);
  Timer* RunAfter(uint64_t micros_delay, TimerProcCallback&& cb);

  Timer* RunEvery(uint64_t micros_interval, const TimerProcCallback& cb);
  Timer* RunEvery(uint64_t micros_interval, TimerProcCallback&& cb);

  void Remove(Timer* timer);

  uint64_t TimeoutMicros() const;
  void RunTimerProcs();

 private:
  typedef std::pair<uint64_t, Timer*> Entry;

  std::vector<Timer*> ExpiredTimers(uint64_t micros);

  std::set<Entry> timers_;

  // No copying allowed
  TimerList(const TimerList&);
  void operator=(const TimerList&);
};

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_TIMERLIST_H_
