// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/mutex.h"

#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "skywalker/logging.h"
#include "util/timeops.h"

namespace skywalker {

static void PthreadCall(const char* label, int result) {
  if (result != 0) {
    LOG_FATAL("%s: %s", label, strerror(result));
  }
}

Mutex::Mutex() {
  PthreadCall("pthread_mutex_init", pthread_mutex_init(&mutex_, nullptr));
}

Mutex::~Mutex() {
  PthreadCall("pthread_mutex_destory", pthread_mutex_destroy(&mutex_));
}

void Mutex::Lock() {
  PthreadCall("pthread_mutex_lock", pthread_mutex_lock(&mutex_));
}

void Mutex::UnLock() {
  PthreadCall("pthread_mutex_unlock", pthread_mutex_unlock(&mutex_));
}

Condition::Condition(Mutex* mutex) : mutex_(mutex) {
#ifdef __linux__
  pthread_condattr_t attr;
  PthreadCall("pthread_condattr_init", pthread_condattr_init(&attr));
  PthreadCall("pthread_condattr_setclock",
              pthread_condattr_setclock(&attr, CLOCK_MONOTONIC));
  PthreadCall("pthread_cond_init", pthread_cond_init(&cond_, &attr));
#else
  PthreadCall("pthread_cond_init", pthread_cond_init(&cond_, nullptr));
#endif
}

Condition::~Condition() {
  PthreadCall("pthread_cond_destory", pthread_cond_destroy(&cond_));
}

void Condition::Wait() {
  PthreadCall("pthread_cond_wait", pthread_cond_wait(&cond_, &mutex_->mutex_));
}

bool Condition::Wait(uint64_t micros) {
  struct timespec outtime;
#ifdef __linux__
  clock_gettime(CLOCK_MONOTONIC, &outtime);
  outtime.tv_sec += micros / 1000000;
  outtime.tv_nsec += (micros % 1000000) * 1000;
  outtime.tv_sec += outtime.tv_nsec / 1000000000;
  outtime.tv_nsec = outtime.tv_nsec % 1000000000;
#else
  uint64_t now = (NowMicros() + micros) * 1000;
  outtime.tv_sec = now / 1000000000;
  outtime.tv_nsec = now % 1000000000;
#endif
  int res = pthread_cond_timedwait(&cond_, &mutex_->mutex_, &outtime);
  if (res != 0 && res != ETIMEDOUT) {
    PthreadCall("pthread_cond_timedwait", res);
  }
  return res == 0;
}

void Condition::Signal() {
  PthreadCall("pthread_cond_signal", pthread_cond_signal(&cond_));
}

void Condition::SignalAll() {
  PthreadCall("pthread_cond_broadcast", pthread_cond_broadcast(&cond_));
}

}  // namespace skywalker
