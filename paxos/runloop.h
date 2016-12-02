#ifndef VOYAGER_PAXOS_RUNLOOP_H_
#define VOYAGER_PAXOS_RUNLOOP_H_

#include <deque>

#include "voyager/paxos/paxos.pb.h"
#include "voyager/port/thread.h"
#include "voyager/port/mutex.h"
#include "voyager/util/slice.h"

namespace voyager {
namespace paxos {

class Instance;

class RunLoop {
 public:
  RunLoop(Instance* instance, const std::string& name = std::string());
  ~RunLoop();

  void Loop();
  void Exit();

  void NewValue(const Slice& value);
  void NewContent(Content* content);

 private:
  void ThreadFunc();

  bool exit_;
  Instance* instance_;
  port::Thread thread_;

  port::Mutex mutex_;
  port::Condition cond_;
  Slice value_;
  std::deque<Content*> contents_;

  // No copying allowed
  RunLoop(const RunLoop&);
  void operator=(const RunLoop&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_RUNLOOP_H_
