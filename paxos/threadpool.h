// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_THREADPOOL_H_
#define SKYWALKER_PAXOS_THREADPOOL_H_

#include <memory>
#include <vector>

#include "util/runloop.h"
#include "util/runloop_thread.h"

namespace skywalker {

class ThreadPool {
 public:
  ThreadPool();

  void Start(uint32_t io_thread_size, uint32_t callback_thread_size,
             uint32_t learn_thread_size, uint32_t clean_thread_size,
             uint32_t master_thread_size);

  RunLoop* GetNextIOLoop();

  RunLoop* GetNextCallbackLoop();

  RunLoop* GetNextLearnLoop();

  RunLoop* GetNextCleanLoop();

  RunLoop* GetNextMasterLoop();

 private:
  bool started_;
  uint32_t io_next_;
  uint32_t callback_next_;
  uint32_t learn_next_;
  uint32_t clean_next_;
  uint32_t master_next_;

  std::vector<RunLoop*> io_loops_;
  std::vector<RunLoop*> callback_loops_;
  std::vector<RunLoop*> learn_loops_;
  std::vector<RunLoop*> clean_loops_;
  std::vector<RunLoop*> master_loops_;

  std::vector<std::unique_ptr<RunLoopThread>> io_threads_;
  std::vector<std::unique_ptr<RunLoopThread>> callback_threads_;
  std::vector<std::unique_ptr<RunLoopThread>> learn_threads_;
  std::vector<std::unique_ptr<RunLoopThread>> clean_threads_;
  std::vector<std::unique_ptr<RunLoopThread>> master_threads_;

  // No copying allowed
  ThreadPool(const ThreadPool&);
  void operator=(const ThreadPool&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_THREADPOOL_H_
