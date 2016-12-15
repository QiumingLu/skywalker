#ifndef VOYAGER_PORT_CURRENTTHREAD_H_
#define VOYAGER_PORT_CURRENTTHREAD_H_

namespace voyager {
namespace port {
namespace CurrentThread {

extern __thread uint64_t  cached_tid;
extern __thread const char* thread_name;
extern void CacheTid();

inline uint64_t Tid() {
  if (__builtin_expect(cached_tid == 0, 0)) {
    CacheTid();
  }
  return cached_tid;
}

inline const char* ThreadName() {
  return thread_name;
}

// extern bool IsMainThread();

}  // namespace CurrentThread
}  // namespace port
}  // namespace voyager

#endif  // VOYAGER_PORT_CURRENTTHREAD_H_
