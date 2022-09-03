// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_PROPOSE_QUEUE_H_
#define SKYWALKER_PAXOS_PROPOSE_QUEUE_H_

#include <mutex>
#include <queue>
#include "skywalker/options.h"
#include "skywalker/state_machine.h"
#include "skywalker/status.h"
#include "util/runloop.h"

namespace skywalker {

class Group;

typedef std::function<void()> ProposeHandler;

class ProposeQueue {
 public:
  explicit ProposeQueue(size_t capacity = 0);
  ~ProposeQueue();

  void SetIOLoop(RunLoop* loop) { io_loop_ = loop; }
  void SetCallbackLoop(RunLoop* loop) { callback_loop_ = loop; }

  bool Put(ProposeHandler f, ProposeCompleteCallback cb);

 private:
  friend class Group;
  bool CheckCapacity() const;
  void ProposeComplete(uint64_t instance_id, const Status& s, void* context);

  size_t capacity_;
  RunLoop* io_loop_;
  RunLoop* callback_loop_;

  std::mutex mutex_;
  bool last_finished_;
  std::queue<ProposeHandler> propose_queue_;
  std::queue<ProposeCompleteCallback> cb_queue_;

  // No copying allowed
  ProposeQueue(const ProposeQueue&);
  void operator=(const ProposeQueue&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_PROPOSE_QUEUE_H_
