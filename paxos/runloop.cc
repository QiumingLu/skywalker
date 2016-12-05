#include "paxos/runloop.h"

#include "paxos/instance.h"
#include "util/mutexlock.h"

namespace skywalker {

void* RunLoop::StartRunLoop(void* data) {
  RunLoop* loop = reinterpret_cast<RunLoop*>(data);
  loop->ThreadFunc();
  return nullptr;
}

RunLoop::RunLoop(Instance* instance)
    : exit_(false),
      instance_(instance),
      thread_(),
      mutex_(),
      cond_(&mutex_),
      value_() {
}

RunLoop::~RunLoop() {
  if (exit_ != true) {
    exit_ = true;
    thread_.Join();
  }
}

void RunLoop::Loop() {
  assert(!thread_.Started());
  thread_.Start(&RunLoop::StartRunLoop, this);
}

void RunLoop::Exit() {
  exit_ = true;
}

void RunLoop::NewValue(const Slice& value) {
  MutexLock lock(&mutex_);
  value_ = value;
  cond_.Signal();
}

void RunLoop::NewContent(Content* content) {
  MutexLock lock(&mutex_);
  contents_.push_back(content);
  cond_.Signal();
}

void RunLoop::ThreadFunc() {
  exit_ = false;
  while(!exit_) {
    Content* content = nullptr;
    {
      MutexLock lock(&mutex_);
      while (contents_.empty() && value_.empty() ) {
        cond_.Wait();
      }
      if (!contents_.empty()) {
        content = contents_.front();
        contents_.pop_front();
      }
    }

    if (content != nullptr) {
      instance_->HandleContent(*content);
      delete content;
    }

    if (!value_.empty()) {
      instance_->HandleNewValue(value_);
      value_.clear();
    }
  }
}

}  // namespace skywalker
