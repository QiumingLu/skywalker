#ifndef SKYWALKER_UTIL_MUTEXLOCK_H_
#define SKYWALKER_UTIL_MUTEXLOCK_H_

namespace skywalker {

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

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_MUTEXLOCK_H_