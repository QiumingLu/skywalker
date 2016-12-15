#ifndef VOYAGER_PORT_MUTEX_H_
#define VOYAGER_PORT_MUTEX_H_

#include <pthread.h>
#include <stdint.h>

namespace voyager {
namespace port {

class Condition;
class Mutex {
 public:
  Mutex();
  ~Mutex();

  void Lock();
  void UnLock();
  void AssertHeld() { }

 private:
  friend class Condition;
  pthread_mutex_t mutex_;

  // No copying allowed
  Mutex(const Mutex&);
  void operator=(const Mutex&);
};

class Condition {
 public:
  explicit Condition(Mutex* mutex);
  ~Condition();

  void Wait();
  bool Wait(uint64_t millisecond);
  void Signal();
  void SignalAll();

 private:
  pthread_cond_t cond_;
  Mutex* mutex_;
};

}  // namespace port
}  // namespace voyager

#endif  // VOYAGER_PORT_MUTEX_H_
