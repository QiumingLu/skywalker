// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_NODE_H_
#define SKYWALKER_INCLUDE_NODE_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "skywalker/options.h"

namespace skywalker {

// Update Makefile if you change these
static const int kMajorVersion = 1;
static const int kMinorVersion = 3;

// A Node is safe for concurrent access from multiple threads without
// any external synchronization.
class Node {
 public:
  // Start the node.
  // Stores a pointer to a heap-allocated node in *nodeptr and returns
  // true on success.
  // Stores nullptr in *nodeptr and returns false on error.
  // Caller should delete *nodeptr when it is no longer needed.
  static bool Start(const Options& options, Node** nodeptr);

  Node() {}
  virtual ~Node() {}

  virtual size_t group_size() const = 0;

  // Propose a new value to the paxos library.
  // If propose success returns true, else returns false.
  // Callback Status::OK() on success.
  // Callback Status::InvalidNode() if the node is not in the membership.
  // Callback Status::Conflict() if there is another value has been chosen.
  // Callback Status::MachineError() if the state machine executed failed.
  // Callback Status::Timeout() if the proposal time is more than a second.
  virtual bool Propose(uint32_t group_id, uint32_t machine_id,
                       const std::string& value, void* context,
                       const ProposeCompleteCallback& cb) = 0;

  virtual bool Propose(uint32_t group_id, uint32_t machine_id,
                       const std::string& value, void* context,
                       ProposeCompleteCallback&& cb) = 0;

  // Change the paxos members.
  // If propose success returns true, else returns false.
  // The callback status like calling Node::Propose().
  virtual bool ChangeMember(uint32_t group_id,
                            const std::vector<std::pair<Member, bool>>& value,
                            void* context,
                            const ProposeCompleteCallback& cb) = 0;

  // Returns the membership.
  virtual void GetMembership(uint32_t group_id, std::vector<Member>* result,
                             uint64_t* version) const = 0;

  // Returns the master.
  // If have master, store the result in *i and return true
  // else return false.
  virtual bool GetMaster(uint32_t group_id, Member* i,
                         uint64_t* version) const = 0;

  // Check whether I'm master or not.
  virtual bool IsMaster(uint32_t group_id) const = 0;

  // Retire master.
  virtual void RetireMaster(uint32_t group_id) = 0;

  // Start to clean the log.
  virtual void StartGC(uint32_t group_id) = 0;

  // Stop to clean the log.
  virtual void StopGC(uint32_t group_id) = 0;

 private:
  // No copying allowed
  Node(const Node&);
  void operator=(const Node&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_NODE_H_
