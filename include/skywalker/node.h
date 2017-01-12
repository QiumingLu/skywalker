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
  // Stores a pointer to a heap-allocated node in *nodeptr and returns
  // true on success.
  // Stores nullptr in *nodeptr and returns false on error.
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
                         int machine_id = -1) = 0;
  virtual Status Propose(uint32_t group_id,
                         const Slice& value,
                         MachineContext* context,
                         uint64_t* instance_id) = 0;

  // Add a new node to the paxos membership.
  // Returns status like calling Node::Propose().
  // It is also returns Status::OK() if the node has already existed
  // in the membership.
  virtual Status AddMember(uint32_t group_id, const IpPort& i) = 0;

  // Remove a node from the paxos membership.
  // Returns status like calling Node::Propose().
  // It is also returns Status::OK() if the node
  // did not exist in the membership.
  virtual Status RemoveMember(uint32_t group_id, const IpPort& i) = 0;

  // Replace an old_node with new_node for the paxos membership.
  // Returns status like calling Node::Propose().
  // It is also returns Status::OK() if the new node has already existed
  // in the membership and the old node did not exist in the membership.
  virtual Status ReplaceMember(uint32_t group_id,
                               const IpPort& new_i,
                               const IpPort& old_i) = 0;

  // Returns the membership.
  virtual void GetMembership(uint32_t group_id,
                             std::vector<IpPort>* result) const = 0;

  // Add a state machine to all groups.
  virtual void AddMachine(StateMachine* machine) = 0;

  // Remove a state machine from all groups.
  virtual void RemoveMachine(StateMachine* machine) = 0;

  // Add a state machine to a specific group.
  virtual void AddMachine(uint32_t group_id, StateMachine* machine) = 0;

  // Remove a state machine from a specific group.
  virtual void RemoveMachine(uint32_t group_id, StateMachine* machine) = 0;

  // Set the master's lease time for all groups.
  // default micros = 10 * 1000 * 1000.
  virtual void SetMasterLeaseTime(uint64_t micros) = 0;

  // Set the master's lease time for a specific group.
  // default micros = 10 * 1000 * 1000.
  virtual void SetMasterLeaseTime(uint32_t group_id, uint64_t micros) = 0;

  // Returns the master.
  // If have master, store the result in *i and return true
  // else return false.
  virtual bool GetMaster(uint32_t group_id,
                         IpPort* i, uint64_t* version) const = 0;

  // Check whether I'm master or not.
  virtual bool IsMaster(uint32_t group_id) const = 0;

  // Retire master.
  virtual void RetireMaster(uint32_t group_id) = 0;

private:
  // No copying allowed
  Node(const Node&);
  void operator=(const Node&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_NODE_H_
