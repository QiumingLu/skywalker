#ifndef VOYAGER_PAXOS_NODE_IMPL_H_
#define VOYAGER_PAXOS_NODE_IMPL_H_

#include <stdint.h>
#include <map>

#include "voyager/paxos/node.h"
#include "voyager/paxos/group.h"
#include "voyager/paxos/options.h"
#include "voyager/paxos/network/network.h"
#include "voyager/util/slice.h"

namespace voyager {
namespace paxos {

class NodeImpl : public Node {
 public:
  NodeImpl(const Options& options);
  virtual ~NodeImpl();

  bool StartWorking();

  virtual bool Propose(uint32_t group_id,
                       const Slice& value,
                       uint64_t* new_instance_id);

 private:
  void OnReceiveMessage(const Slice& s);

  const Options options_;

  Network network_;

  std::map<uint32_t, Group*> groups_;

  // No copying allowed
  NodeImpl(const NodeImpl&);
  void operator=(const NodeImpl&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_NODE_IMPL_H_
