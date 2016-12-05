#include "util/thread.h"

#include <assert.h>
#include <string.h>

#include "skywalker/logging.h"

namespace skywalker {

Thread::Thread()
     : started_(false),
       joined_(false),
       thread_(0) {
}

Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(thread_);
  }
}

void Thread::Start(void* (*function)(void* arg), void* arg) {
  assert(!started_);
  started_ = true;
  PthreadCall("pthread_create",
              pthread_create(&thread_, nullptr, function, arg));
}

void Thread::Join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  PthreadCall("pthread_join", pthread_join(thread_, nullptr));
}

void Thread::PthreadCall(const char* label, int result) {
  if (result != 0) {
    Log(LOG_FATAL, "%s: %s\n", label, strerror(result));
  }
}

}  // namespace skywalker
