#ifndef SKYWALKER_PAXOS_RUNLOOP_H_
#define SKYWALKER_PAXOS_RUNLOOP_H_

#include <deque>

#include "paxos/paxos.pb.h"
#include "skywalker/slice.h"
#include "util/thread.h"
#include "util/mutex.h"

namespace skywalker {

class Instance;

class RunLoop {
 public:
  RunLoop(Instance* instance);
  ~RunLoop();

  void Loop();
  void Exit();

  void NewValue(const Slice& value);
  void NewContent(Content* content);

 private:
  static void* StartRunLoop(void* data);
  void ThreadFunc();

  bool exit_;
  Instance* instance_;
  Thread thread_;

  Mutex mutex_;
  Condition cond_;
  std::deque<std::string*> values_;
  std::deque<Content*> contents_;

  // No copying allowed
  RunLoop(const RunLoop&);
  void operator=(const RunLoop&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_RUNLOOP_H_
