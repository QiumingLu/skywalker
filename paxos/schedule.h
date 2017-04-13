// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_SCHEDULE_H_
#define SKYWALKER_PAXOS_SCHEDULE_H_

#include <memory>

#include "util/runloop.h"
#include "util/runloop_thread.h"

namespace skywalker {

class Schedule {
 public:
  Schedule(bool use_master = true);

  void Start();

  RunLoop* MasterLoop() const;

  RunLoop* LearnLoop() const;

  RunLoop* IOLoop() const;

  RunLoop* CallbackLoop() const;

 private:
  bool use_master_;

  RunLoop* callback_loop_;
  RunLoop* io_loop_;
  RunLoop* learn_loop_;
  RunLoop* master_loop_;

  RunLoopThread callback_thread_;
  RunLoopThread io_thread_;
  RunLoopThread learn_thread_;
  RunLoopThread master_thread_;

  // No copying allowed
  Schedule(const Schedule&);
  void operator=(const Schedule&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_SCHEDULE_H_
