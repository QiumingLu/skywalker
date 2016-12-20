#ifndef SKYWALKER_INCLUDE_NODE_H_
#define SKYWALKER_INCLUDE_NODE_H_

#include <stdint.h>

#include "skywalker/options.h"
#include "skywalker/slice.h"
#include "skywalker/state_machine.h"

namespace skywalker {

class Node {
 public:
  static bool Start(const Options& options,
                    Node** nodeptr);

  Node() { }
  virtual ~Node() { }

  virtual int Propose(uint32_t group_id,
                      const Slice& value,
                      uint64_t* instance_id,
                      int machine_id = -1) = 0;

  virtual void AddMachine(uint32_t group_id, StateMachine* machine) = 0;
  virtual void RemoveMachine(uint32_t group_id, StateMachine* machine) = 0;

  virtual void AddMember(uint32_t group_id, const IpPort& i) = 0;
  virtual void RemoveMember(uint32_t group_id, const IpPort& i) = 0;

private:
  // No copying allowed
  Node(const Node&);
  void operator=(const Node&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_NODE_H_
