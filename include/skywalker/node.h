// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_NODE_H_
#define SKYWALKER_INCLUDE_NODE_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "skywalker/options.h"
#include "skywalker/slice.h"
#include "skywalker/status.h"

namespace skywalker {

// Update Makefile if you change these
static const int kMajorVersion = 1;
static const int kMinorVersion = 2;

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

  virtual size_t group_size() const = 0;

  // Propose a new value to the paxos library.
  // If propose success returns true, else returns false.
  // Callback Status::OK() on success.
  // Callback Status::InvalidNode() if the node is not in the membership.
  // Callback Status::Conflict() if there is another value has been chosen.
  // Callback Status::MachineError() if the state machine executed failed.
  // Callback Status::Timeout() if the proposal time is more than a second.
  virtual bool Propose(uint32_t group_id,
                       const std::string& value,
                       MachineContext* context,
                       const ProposeCompleteCallback& cb) = 0;

  virtual bool Propose(uint32_t group_id,
                       const std::string& value,
                       MachineContext* context,
                       ProposeCompleteCallback&& cb) = 0;

  // Add a new node to the paxos membership.
  // If propose success returns true, else returns false.
  // The callback status like calling Node::Propose().
  // It is also callback Status::OK() if the node has already existed
  // in the membership.
  virtual bool AddMember(uint32_t group_id, const IpPort& i,
                         const MembershipCompleteCallback& cb) = 0;

  // Remove a node from the paxos membership.
  // If propose success returns true, else returns false.
  // The Callback status like calling Node::Propose().
  // It is also callback Status::OK() if the node
  // did not exist in the membership.
  virtual bool RemoveMember(uint32_t group_id, const IpPort& i,
                            const MembershipCompleteCallback& cb) = 0;

  // Replace an old_node with new_node for the paxos membership.
  // If propose success returns true, else returns false.
  // The Callback status like calling Node::Propose().
  // It is also callback Status::OK() if the new node has already existed
  // in the membership and the old node did not exist in the membership.
  virtual bool ReplaceMember(uint32_t group_id,
                             const IpPort& new_i,
                             const IpPort& old_i,
                             const MembershipCompleteCallback& cb) = 0;

  // Returns the membership.
  virtual void GetMembership(uint32_t group_id,
                             std::vector<IpPort>* result) const = 0;

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
