#ifndef SKYWALKER_PAXOS_GROUP_H_
#define SKYWALKER_PAXOS_GROUP_H_

#include <memory>

#include "paxos/config.h"
#include "paxos/instance.h"
#include "paxos/paxos.pb.h"
#include "skywalker/options.h"
#include "skywalker/slice.h"
#include "util/mutex.h"

namespace skywalker {

class Network;

class Group {
 public:
  Group(uint32_t group_id, uint64_t node_id,
        const Options& options, Network* network);

  bool Start();

  void AddMachine(StateMachine* machine);
  void RemoveMachine(StateMachine* machine);

  int OnPropose(const Slice& value,
                       uint64_t* instance_id,
                       int machine_id = -1);

  void OnReceiveContent(const std::shared_ptr<Content>& c);

 private:
  void ProposeComplete(int result, uint64_t instance_id);

  Config config_;
  Instance instance_;
  RunLoop* loop_;

  Mutex mutex_;
  Condition cond_;
  bool propose_end_;
  uint64_t instance_id_;
  int result_;

  // No copying allowed
  Group(const Group&);
  void operator=(const Group&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_GROUP_H_
