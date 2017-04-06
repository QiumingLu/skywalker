// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_PROPOSE_QUEUE_H_
#define SKYWALKER_PAXOS_PROPOSE_QUEUE_H_

#include <queue>
#include "util/thread.h"
#include "util/mutex.h"
#include "util/mutexlock.h"
#include "skywalker/status.h"
#include "skywalker/state_machine.h"
#include "skywalker/options.h"

namespace skywalker {

class Group;
class Schedule;

typedef std::function<void ()> ProposeHandler;

class ProposeQueue {
 public:
  explicit ProposeQueue(Schedule* schedule);
  ~ProposeQueue();

  void SetCapacity(size_t capacity) { capacity_ = capacity; }

  bool Put(const ProposeHandler& f, const ProposeCompleteCallback& cb);
  bool Put(ProposeHandler&& f, const ProposeCompleteCallback& cb);
  bool Put(const ProposeHandler& f, ProposeCompleteCallback&& cb);
  bool Put(ProposeHandler&& f, ProposeCompleteCallback&& cb);

 private:
  friend class Group;
  static void* StartWorking(void* data);
  bool CheckCapacity() const;
  void Propose();
  void ProposeComplete(MachineContext* context,
                       const Status& s, uint64_t instance_id);

  Schedule* schedule_;
  bool exit_;
  Thread thread_;

  Mutex mutex_;
  Condition cond_;
  bool has_cb_;
  size_t capacity_;
  std::queue<ProposeHandler> propose_queue_;
  std::queue<ProposeCompleteCallback> cb_queue_;

  // No copying allowed
  ProposeQueue(const ProposeQueue&);
  void operator=(const ProposeQueue&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_PROPOSE_QUEUE_H_
