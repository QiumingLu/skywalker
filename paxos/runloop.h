#ifndef SKYWALKER_PAXOS_RUNLOOP_H_
#define SKYWALKER_PAXOS_RUNLOOP_H_

#include <deque>

#include "paxos/paxos.pb.h"
#include "skywalker/slice.h"
#include "util/thread.h"
#include "util/mutex.h"
#include "util/timerlist.h"

namespace skywalker {

class Instance;

class RunLoop {
 public:
  RunLoop();
  ~RunLoop();

  void SetInstance(Instance* instance) { instance_ = instance; }

  void Loop();
  void Exit();

  void NewValue(const Slice& value);
  void NewContent(Content* content);

  TimerList::Timer* RunAt(uint64_t milli_value,
                          const std::function<void ()>& cb);
  TimerList::Timer* RunAfter(uint64_t milli_delay,
                             const std::function<void ()>& cb);
  TimerList::Timer* RunEvery(uint64_t milli_interval,
                             const std::function<void ()>& cb);

  void Remove(TimerList::Timer* t);

 private:
  static void* StartRunLoop(void* data);
  void ThreadFunc();

  bool exit_;
  Instance* instance_;
  Thread thread_;

  Mutex mutex_;
  Condition cond_;
  std::deque<std::string*> values_;
  std::deque<Content*> contents_;

  TimerList timers_;

  // No copying allowed
  RunLoop(const RunLoop&);
  void operator=(const RunLoop&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_RUNLOOP_H_
