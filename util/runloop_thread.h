// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_RUNLOOP_THREAD_H_
#define SKYWALKER_UTIL_RUNLOOP_THREAD_H_

#include "util/mutex.h"
#include "util/thread.h"
#include "util/runloop.h"

namespace skywalker {

class RunLoopThread {
 public:
  explicit RunLoopThread();
  ~RunLoopThread();

  RunLoop* Loop();

 private:
  static void* StartRunLoop(void* data);

  void ThreadFunc();

  RunLoop *loop_;
  Mutex mu_;
  Condition cond_;
  Thread thread_;

  // No copying allowed
  RunLoopThread(const RunLoopThread&);
  void operator=(const RunLoopThread&);
};

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_RUNLOOP_THREAD_H_
