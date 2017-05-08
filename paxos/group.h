// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_GROUP_H_
#define SKYWALKER_PAXOS_GROUP_H_

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include "util/mutex.h"
#include "paxos/config.h"
#include "paxos/instance.h"
#include "paxos/schedule.h"
#include "paxos/propose_queue.h"
#include "proto/paxos.pb.h"
#include "skywalker/options.h"
#include "machine/membership_machine.h"
#include "machine/master_machine.h"

namespace skywalker {

class Network;

class Group {
 public:
  Group(uint64_t node_id, const GroupOptions& options, Network* network);

  bool Start();

  void SyncMembership();
  void SyncMaster();

  bool OnPropose(const std::string& value, MachineContext* context,
                 const ProposeCompleteCallback& cb);

  bool OnPropose(const std::string& value, MachineContext* context,
                 ProposeCompleteCallback&& cb);

  void OnReceiveContent(const std::shared_ptr<Content>& c);

  bool AddMember(const IpPort& ip, const MembershipCompleteCallback& cb);
  bool RemoveMember(const IpPort& ip, const MembershipCompleteCallback& cb);
  bool ReplaceMember(const IpPort& new_i, const IpPort& old_i,
                     const MembershipCompleteCallback& cb);
  void GetMembership(std::vector<IpPort>* result) const;

  void SetMasterLeaseTime(uint64_t micros);
  bool GetMaster(IpPort* i, uint64_t* version) const;
  bool IsMaster() const;
  void RetireMaster();

 private:
  void SyncMembershipInLoop(MachineContext* context);
  void TryBeMaster();
  void TryBeMasterInLoop(MachineContext* context);
  void AddMemberInLoop(uint64_t node_id, MachineContext* context);
  void RemoveMemberInLoop(uint64_t node_id, MachineContext* context);
  void ReplaceMemberInLoop(uint64_t new_node_id, uint64_t old_node_id,
                           MachineContext* context);
  bool NewPropose(ProposeHandler&& f);
  void ProposeComplete(MachineContext* context,
                       const Status& result, uint64_t instance_id);

  const uint64_t node_id_;
  Config config_;
  Instance instance_;

  uint64_t lease_timeout_;
  bool retrie_master_;
  MembershipMachine* membership_machine_;
  MasterMachine* master_machine_;
  ProposeCompleteCallback propose_cb_;

  Mutex mutex_;
  Condition cond_;
  bool propose_end_;
  Status result_;

  ProposeQueue propose_queue_;
  std::unique_ptr<Schedule> schedule_;

  // No copying allowed
  Group(const Group&);
  void operator=(const Group&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_GROUP_H_
