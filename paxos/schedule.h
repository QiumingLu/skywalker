// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_SCHEDULE_H_
#define SKYWALKER_PAXOS_SCHEDULE_H_

#include <memory>
#include <vector>

#include "util/runloop.h"
#include "util/runloop_thread.h"

namespace skywalker {

class ThreadPool {
 public:
  ThreadPool();

  void Start(uint32_t io_thread_size, uint32_t callback_thread_size);

  RunLoop* GetNextIOLoop();

  RunLoop* GetNextCallbackLoop();

 private:
  bool started_;
  uint32_t io_next_;
  uint32_t callback_next_;

  std::vector<RunLoop*> io_loops_;
  std::vector<RunLoop*> callback_loops_;

  std::vector<std::unique_ptr<RunLoopThread>> io_threads_;
  std::vector<std::unique_ptr<RunLoopThread>> callback_threads_;

  // No copying allowed
  ThreadPool(const ThreadPool&);
  void operator=(const ThreadPool&);
};

class Schedule {
 public:
  static Schedule* Instance() {
    static Schedule schedule;
    return &schedule;
  }

  RunLoop* CleanLoop() const;

  RunLoop* LearnLoop() const;

  RunLoop* MasterLoop();

 private:
  RunLoop* clean_loop_;
  RunLoop* learn_loop_;
  RunLoop* master_loop_;

  RunLoopThread clean_thread_;
  RunLoopThread learn_thread_;
  RunLoopThread master_thread_;

  Schedule();
  ~Schedule();
  Schedule(const Schedule&);
  void operator=(const Schedule&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_SCHEDULE_H_
