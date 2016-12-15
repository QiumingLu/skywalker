#ifndef VOYAGER_PORT_MUTEXLOCK_H_
#define VOYAGER_PORT_MUTEXLOCK_H_

#include "voyager/port/mutex.h"

namespace voyager {
namespace port {

class MutexLock {
 public:
  explicit MutexLock(Mutex* mutex)
      : mutex_(mutex) {
    mutex_->Lock();
  }

  ~MutexLock() { mutex_->UnLock(); }

 private:
  Mutex* const mutex_;

  // No copying allowed
  MutexLock(const MutexLock&);
  void operator=(const MutexLock&);
};

}  // namespace port
}  // namespace voyager

#endif  // VOYAGER_PORT_MUTEXLOCK_H_
