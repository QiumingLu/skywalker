// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_RUNLOOP_H_
#define SKYWALKER_PAXOS_RUNLOOP_H_

#include <vector>
#include <functional>

#include "proto/paxos.pb.h"
#include "skywalker/slice.h"
#include "util/thread.h"
#include "util/mutex.h"
#include "util/timerlist.h"

namespace skywalker {

class Instance;

class RunLoop {
 public:
  typedef std::function<void ()> Func;

  RunLoop();
  ~RunLoop();

  void Loop();
  void Exit();

  void QueueInLoop(const Func& func);
  void QueueInLoop(Func&& func);

  TimerId RunAt(uint64_t micros_value, const TimerProcCallback& cb);
  TimerId RunAfter(uint64_t micros_delay, const TimerProcCallback& cb);
  TimerId RunEvery(uint64_t micros_interval, const TimerProcCallback& cb);

  TimerId RunAt(uint64_t micros_value, TimerProcCallback&& cb);
  TimerId RunAfter(uint64_t micros_delay, TimerProcCallback&& cb);
  TimerId RunEvery(uint64_t micros_interval, TimerProcCallback&& cb);

  void Remove(const TimerId& t);

 private:
  static void* StartRunLoop(void* data);
  void ThreadFunc();

  bool exit_;
  Instance* instance_;
  Thread thread_;

  Mutex mutex_;
  Condition cond_;
  std::vector<Func> funcs_;
  TimerList timers_;

  // No copying allowed
  RunLoop(const RunLoop&);
  void operator=(const RunLoop&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_RUNLOOP_H_
