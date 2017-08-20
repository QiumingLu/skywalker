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

class Schedule {
 public:
  static Schedule* Instance() {
    static Schedule schedule;
    return &schedule;
  }

  void Start(uint32_t io_thread_size, bool use_master = true);

  RunLoop* CallbackLoop() const;

  RunLoop* CleanLoop() const;

  RunLoop* LearnLoop() const;

  RunLoop* MasterLoop() const;

  // round-robin
  RunLoop* GetNextIOLoop();

 private:
  bool started_;
  uint32_t next_;

  RunLoop* callback_loop_;
  RunLoop* learn_loop_;
  RunLoop* clean_loop_;
  RunLoop* master_loop_;

  RunLoopThread callback_thread_;
  RunLoopThread clean_thread_;
  RunLoopThread learn_thread_;
  RunLoopThread master_thread_;

  std::vector<RunLoop*> io_loops_;
  std::vector<std::unique_ptr<RunLoopThread> > io_threads_;

  Schedule();
  ~Schedule();
  Schedule(const Schedule&);
  void operator=(const Schedule&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_SCHEDULE_H_
