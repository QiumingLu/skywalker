#ifndef SKYWALKER_PAXOS_PROPOSE_QUEUE_H_
#define SKYWALKER_PAXOS_PROPOSE_QUEUE_H_

#include <queue>
#include "util/thread.h"
#include "util/mutex.h"
#include "util/mutexlock.h"
#include "paxos/config.h"
#include "skywalker/status.h"
#include "skywalker/state_machine.h"

namespace skywalker {

class Group;

typedef std::function<void ()> ProposeHandler;

class ProposeQueue {
 public:
  ProposeQueue(Config* config);
  ~ProposeQueue();

  void Put(const ProposeHandler& f, const ProposeCompleteCallback& cb);

 private:
  friend class Group;
  static void* StartWorking(void* data);
  void Propose();
  void ProposeComplete(MachineContext* context, const Status& s);

  Config* config_;
  bool exit_;
  Thread thread_;

  Mutex mutex_;
  Condition cond_;
  bool has_cb_;
  std::queue<ProposeHandler> propose_queue_;
  std::queue<ProposeCompleteCallback> cb_queue_;

  // No copying allowed
  ProposeQueue(const ProposeQueue&);
  void operator=(const ProposeQueue&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_PROPOSE_QUEUE_H_
