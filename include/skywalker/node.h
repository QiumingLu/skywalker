#ifndef SKYWALKER_INCLUDE_NODE_H_
#define SKYWALKER_INCLUDE_NODE_H_

#include <stdint.h>

#include "skywalker/options.h"
#include "skywalker/slice.h"
#include "skywalker/state_machine.h"
#include "skywalker/status.h"

namespace skywalker {

// A Node is safe for concurrent access from multiple threads without
// any external synchronization.
class Node {
 public:
  // Start the node.
  // Store a pointer to a heap-allocated node in *nodeptr and returns
  // true on success.
  // Store nullptr in *nodeptr and returns false on error.
  // Caller should delete *nodeptr when it is no longer needed.
  static bool Start(const Options& options,
                    Node** nodeptr);

  Node() { }
  virtual ~Node() { }

  // Propose a new value to the paxos library.
  // Returns Status::OK() on success.
  // Returns Status::InvalidNode() if the node is not in the membership.
  // Returns Status::Conflict() if there is another value has been chosen.
  // Returns Status::MachineError() if the state machine executed failed.
  // Returns Status::Timeout() if the proposal time is more than a second.
  virtual Status Propose(uint32_t group_id,
                         const Slice& value,
                         uint64_t* instance_id,
                         int machine_id = -1) = 0;

  // Add a new node to the paxos membership.
  // Returns Status::Unavailable() if the membership has's been synchronized.
  // Returns Status::AlreadyExists() if the node has already existed
  // in the membership.
  // Returns some other Status like calling Node::Propose().
  virtual Status AddMember(uint32_t group_id, const IpPort& i) = 0;

  // Remove a node from the paxos membership.
  // Returns Status::Unavailable() if the membership has's been synchronized.
  // Returns Status::NotFound() if the node is not found in the membership.
  // Returns some other Status like calling Node::Propose().
  virtual Status RemoveMember(uint32_t group_id, const IpPort& i) = 0;

  // Replace an old_node with new_node for the paxos membership.
  // Returns Status::Unavailable() if the membership has's been synchronized.
  // Returns some other Status like calling Node::Propose().
  virtual Status ReplaceMember(uint32_t group_id,
                               const IpPort& new_i,
                               const IpPort& old_i) = 0;

  // Add a state machine to all groups.
  virtual void AddMachine(StateMachine* machine) = 0;

  // Remove a state machine from all groups.
  virtual void RemoveMachine(StateMachine* machine) = 0;

  // Add a state machine to a specific group.
  virtual void AddMachine(uint32_t group_id, StateMachine* machine) = 0;

  // Remove a state machine from a specific group.
  virtual void RemoveMachine(uint32_t group_id, StateMachine* machine) = 0;

private:
  // No copying allowed
  Node(const Node&);
  void operator=(const Node&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_NODE_H_
