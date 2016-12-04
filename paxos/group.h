#ifndef SKYWALKER_PAXOS_GROUP_H_
#define SKYWALKER_PAXOS_GROUP_H_

#include "paxos/config.h"
#include "paxos/instance.h"
#include "paxos/paxos.pb.h"
#include "skywalker/state_machine.h"
#include "skywalker/options.h"
#include "skywalker/slice.h"

namespace skywalker {

class Network;

class Group {
 public:
  Group(uint32_t group_id, const Options& options, Network* network);

  bool Start();

  Instance* GetInstance() { return &instance_; }

  bool OnReceiveValue(const Slice& value,
                      MachineContext* context,
                      uint64_t* new_instance_id);

  void OnReceiveContent(Content* content);

 private:
  Config config_;
  Instance instance_;

  // No copying allowed
  Group(const Group&);
  void operator=(const Group&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_GROUP_H_
