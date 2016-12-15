#ifndef VOYAGER_CORE_SCHEDULE_H_
#define VOYAGER_CORE_SCHEDULE_H_

#include <memory>
#include <vector>

#include "voyager/core/bg_eventloop.h"

namespace voyager {

class Schedule {
 public:
  Schedule(EventLoop* ev, int size);

  void Start();

  EventLoop* AssignLoop();

  bool Started() const { return started_; }

 private:
  EventLoop* baseloop_;
  size_t size_;
  bool started_;
  std::vector<std::unique_ptr<BGEventLoop>> bg_loops_;
  std::vector<EventLoop*> loops_;

  // No copying alloweded
  Schedule(const Schedule&);
  void operator=(const Schedule&);
};

}  // namespace voyager

#endif  // VOYAGER_CORE_SCHEDULE_H_
