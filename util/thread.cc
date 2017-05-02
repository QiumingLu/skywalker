// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/thread.h"

#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <sys/syscall.h>

#ifdef __linux__
#include <sys/prctl.h>
#endif

#include "skywalker/logging.h"

namespace skywalker {

namespace CurrentThread {

__thread uint64_t cached_tid = 0;

}  // namespace CurrentThread

namespace {

#ifdef __linux__
uint64_t GetTid() {
  return static_cast<uint64_t>(::syscall(SYS_gettid));
}

#elif __APPLE__
uint64_t GetTid() {
  return static_cast<uint64_t>(pthread_mach_thread_np(pthread_self()));
}

#else
uint64_t GetTid() {
  pthread_t tid = pthread_self();
  uint64_t thread_id = 0;
  memcpy(&thread_id, &tid, std::min(sizeof(thread_id), sizeof(tid)));
  return thread_id;
}

#endif

void AfterFork() {
  CurrentThread::cached_tid = 0;
  CurrentThread::Tid();
}

class ThreadInitializer {
 public:
  ThreadInitializer() {
    CurrentThread::Tid();
    pthread_atfork(nullptr, nullptr, &AfterFork);
  }
};

ThreadInitializer thread_init;

}

namespace CurrentThread {

uint64_t Tid() {
  if (cached_tid == 0) {
    cached_tid = GetTid();
  }
  return cached_tid;
}

}

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
    LOG_FATAL("%s: %s", label, strerror(result));
  }
}

}  // namespace skywalker
