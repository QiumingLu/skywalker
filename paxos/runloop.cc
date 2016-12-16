#include "paxos/runloop.h"

#include <algorithm>

#include "paxos/instance.h"
#include "util/mutexlock.h"

namespace skywalker {

void* RunLoop::StartRunLoop(void* data) {
  RunLoop* loop = reinterpret_cast<RunLoop*>(data);
  loop->ThreadFunc();
  return nullptr;
}

RunLoop::RunLoop()
    : exit_(false),
      instance_(nullptr),
      thread_(),
      mutex_(),
      cond_(&mutex_) {
}

RunLoop::~RunLoop() {
  if (exit_ != true) {
    exit_ = true;
    thread_.Join();
  }
}

void RunLoop::Loop() {
  assert(!thread_.Started());
  thread_.Start(&RunLoop::StartRunLoop, this);
}

void RunLoop::Exit() {
  exit_ = true;
}

void RunLoop::QueueInLoop(const Func& func) {
  MutexLock lock(&mutex_);
  funcs_.push_back(func);
  cond_.Signal();
}

void RunLoop::QueueInLoop(Func&& func) {
  MutexLock lock(&mutex_);
  funcs_.push_back(std::move(func));
  cond_.Signal();
}

void RunLoop::ThreadFunc() {
  exit_ = false;
  while(!exit_) {
    uint64_t t = timers_.TimeoutMicros() / 1000;
    uint64_t timeout = std::min(t, static_cast<uint64_t>(5000));
    std::vector<Func> funcs;
    {
      MutexLock lock(&mutex_);
      bool wait = true;
      while (funcs_.empty() && wait) {
        cond_.Wait(timeout);
        wait = false;
      }
      funcs.swap(funcs_);
    }
    for (auto f : funcs) {
      f();
    }
    timers_.RunTimerProcs();
  }
}

TimerId RunLoop::RunAt(uint64_t milli_value,
                       const TimerProcCallback& cb) {
  return timers_.RunAt(milli_value*1000, cb);
}

TimerId RunLoop::RunAfter(uint64_t milli_delay,
                          const TimerProcCallback& cb) {
  return timers_.RunAfter(milli_delay*1000, cb);
}

TimerId RunLoop::RunEvery(uint64_t milli_interval,
                          const TimerProcCallback& cb) {
  return timers_.RunEvery(milli_interval*1000, cb);
}

TimerId RunLoop::RunAt(uint64_t milli_value,
                       TimerProcCallback&& cb) {
  return timers_.RunAt(milli_value*1000, std::move(cb));
}

TimerId RunLoop::RunAfter(uint64_t milli_delay,
                          TimerProcCallback&& cb) {
  return timers_.RunAfter(milli_delay*1000, std::move(cb));
}

TimerId RunLoop::RunEvery(uint64_t milli_interval,
                          TimerProcCallback&& cb) {
  return timers_.RunEvery(milli_interval*1000, std::move(cb));
}

void RunLoop::Remove(const TimerId& t) {
  timers_.Remove(t);
}

}  // namespace skywalker
