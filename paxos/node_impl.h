// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_NODE_IMPL_H_
#define SKYWALKER_PAXOS_NODE_IMPL_H_

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "paxos/group.h"
#include "network/network.h"
#include "skywalker/slice.h"
#include "skywalker/status.h"
#include "skywalker/options.h"
#include "skywalker/node.h"

namespace skywalker {

class NodeImpl : public Node {
 public:
  explicit NodeImpl(const Options& options);
  virtual ~NodeImpl();

  bool StartWorking();

  virtual bool Propose(uint32_t group_id,
                       const std::string& value,
                       MachineContext* context,
                       const ProposeCompleteCallback& cb);

  virtual bool Propose(uint32_t group_id,
                       const std::string& value,
                       MachineContext* context,
                       ProposeCompleteCallback&& cb);

  virtual bool AddMember(uint32_t group_id, const IpPort& i,
                         const MembershipCompleteCallback& cb);
  virtual bool RemoveMember(uint32_t group_id, const IpPort& i,
                            const MembershipCompleteCallback& cb);
  virtual bool ReplaceMember(uint32_t group_id,
                             const IpPort& new_i, const IpPort& old_i,
                             const MembershipCompleteCallback& cb);
  virtual void GetMembership(uint32_t group_id,
                             std::vector<IpPort>* result) const;

  virtual void AddMachine(StateMachine* machine);
  virtual void RemoveMachine(StateMachine* machine);

  virtual void AddMachine(uint32_t group_id, StateMachine* machine);
  virtual void RemoveMachine(uint32_t group_id, StateMachine* machine);

  virtual void SetMasterLeaseTime(uint64_t micros);
  virtual void SetMasterLeaseTime(uint32_t group_id, uint64_t micros);
  virtual bool GetMaster(uint32_t group_id,
                         IpPort* i, uint64_t* version) const;
  virtual bool IsMaster(uint32_t group_id) const;
  virtual void RetireMaster(uint32_t group_id);

 private:
  void OnReceiveMessage(const Slice& s);

  bool stop_;
  const Options options_;
  uint64_t node_id_;
  Network network_;
  std::map<uint32_t, std::unique_ptr<Group> > groups_;

  // No copying allowed
  NodeImpl(const NodeImpl&);
  void operator=(const NodeImpl&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_NODE_IMPL_H_
