#ifndef VOYAGER_CORE_EVENT_SELECT_H_
#define VOYAGER_CORE_EVENT_SELECT_H_

#include <sys/select.h>
#include <sys/types.h>

#include <map>
#include <vector>

#include "voyager/core/event_poller.h"

namespace voyager {

class EventSelect : public EventPoller{
 public:
  explicit EventSelect(EventLoop* ev);
  virtual ~EventSelect();
  virtual void Poll(int timeout, std::vector<Dispatch*>* dispatches);
  virtual void RemoveDispatch(Dispatch* dispatch);
  virtual void UpdateDispatch(Dispatch* dispatch);

 private:
  int nfds_;
  fd_set readfds_;
  fd_set writefds_;
  fd_set exceptfds_;
  std::map<int, Dispatch*> worker_map_;
};

}  // namespace voyager

#endif  // VOYAGER_CORE_EVENT_SELECT_H_
