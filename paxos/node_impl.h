#ifndef SKYWALKER_PAXOS_NODE_IMPL_H_
#define SKYWALKER_PAXOS_NODE_IMPL_H_

#include <stdint.h>
#include <map>

#include "paxos/group.h"
#include "network/network.h"
#include "skywalker/slice.h"
#include "skywalker/options.h"
#include "skywalker/node.h"

namespace skywalker {

class NodeImpl : public Node {
 public:
  explicit NodeImpl(const Options& options);
  virtual ~NodeImpl();

  bool StartWorking();

  virtual int Propose(uint32_t group_id,
                      const Slice& value,
                      uint64_t* instance_id,
                      int machine_id = -1);

  virtual void AddMachine(uint32_t group_id, StateMachine* machine);
  virtual void RemoveMachine(uint32_t group_id, StateMachine* machine);

  virtual void AddMember(uint32_t group_id, const IpPort& i);
  virtual void RemoveMember(uint32_t group_id, const IpPort& i);

 private:
  void OnReceiveMessage(const Slice& s);

  const Options options_;
  uint64_t node_id_;
  Network network_;
  std::map<uint32_t, Group*> groups_;

  // No copying allowed
  NodeImpl(const NodeImpl&);
  void operator=(const NodeImpl&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_NODE_IMPL_H_
