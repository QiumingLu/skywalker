#ifndef VOYAGER_PAXOS_GROUP_H_
#define VOYAGER_PAXOS_GROUP_H_

#include "voyager/paxos/config.h"
#include "voyager/paxos/instance.h"
#include "voyager/paxos/state_machine.h"
#include "voyager/paxos/options.h"
#include "voyager/paxos/paxos.pb.h"
#include "voyager/util/slice.h"

namespace voyager {
namespace paxos {

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

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_GROUP_H_
