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
      cond_(&mutex_) {
}

RunLoop::~RunLoop() {
  if (exit_ != true) {
    exit_ = true;
    thread_.Join();
  }

  for (auto v : values_) {
    delete v;
  }
  for (auto c : contents_) {
    delete c;
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
  values_.push_back(new std::string(value.data(), value.size()));
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
    std::string* value = nullptr;
    {
      MutexLock lock(&mutex_);
      while (contents_.empty() && values_.empty()) {
        cond_.Wait();
      }
      if (!contents_.empty()) {
        content = contents_.front();
        contents_.pop_front();
      }

      if (!values_.empty()) {
        value = values_.front();
        values_.pop_front();
      }
    }

    if (content != nullptr) {
      instance_->HandleContent(*content);
      delete content;
    }

    if (value != nullptr) {
      instance_->HandleNewValue(*value);
      delete value;
    }
  }
}

}  // namespace skywalker
