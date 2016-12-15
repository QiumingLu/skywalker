#ifndef VOYAGER_PORT_THREAD_H_
#define VOYAGER_PORT_THREAD_H_

#include <pthread.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <string>
#include <utility>


namespace voyager {
namespace port {

class Thread {
 public:
  typedef std::function<void()> ThreadFunc;
  explicit Thread(const ThreadFunc& func,
                  const std::string& name = std::string());
  explicit Thread(ThreadFunc&& func,
                  const std::string& name = std::string());
  ~Thread();

  void Start();
  void Join();

  bool Started() const { return started_; }
  bool Joined() const { return joined_; }
  uint64_t Tid() const { return tid_; }
  const std::string& Name() const { return name_; }
  static int ThreadCreatedNum() {
    return num_.load(std::memory_order_relaxed);
  }

 private:
  void SetDefaultName();
  void PthreadCall(const char* label, int result);

  bool started_;
  bool joined_;
  pthread_t pthread_id_;
  uint64_t tid_;
  ThreadFunc func_;
  std::string name_;
  static std::atomic<int> num_;

  // No copying allowed
  Thread(const Thread&);
  void operator=(const Thread&);
};

}  // namespace port
}  // namespace voyager

#endif  // VOYAGER_PORT_THREAD_H_
