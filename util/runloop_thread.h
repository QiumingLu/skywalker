// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_RUNLOOP_THREAD_H_
#define SKYWALKER_UTIL_RUNLOOP_THREAD_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include "util/runloop.h"

namespace skywalker {

class RunLoopThread {
 public:
  RunLoopThread();
  ~RunLoopThread();

  RunLoop* Loop();

 private:
  void ThreadFunc();

  RunLoop* loop_;
  std::mutex mu_;
  std::condition_variable cond_;
  std::unique_ptr<std::thread> thread_;

  // No copying allowed
  RunLoopThread(const RunLoopThread&);
  void operator=(const RunLoopThread&);
};

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_RUNLOOP_THREAD_H_
