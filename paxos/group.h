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

  bool AddMember(const Member& i, const MembershipCompleteCallback& cb);
  bool RemoveMember(const Member& i, const MembershipCompleteCallback& cb);
  bool ReplaceMember(const Member& i, const Member& j,
                     const MembershipCompleteCallback& cb);
  void GetMembership(std::vector<Member>* result, uint64_t* version) const;

  void SetMasterLeaseTime(uint64_t micros);
  bool GetMaster(Member* i, uint64_t* version) const;
  bool IsMaster() const;
  void RetireMaster();

  void StartGC();
  void StopGC();

 private:
  void SyncMembershipInLoop(MachineContext* context);
  void TryBeMaster();
  void TryBeMasterInLoop(MachineContext* context);
  void AddMemberInLoop(const Member& i, MachineContext* context);
  void RemoveMemberInLoop(const Member& i, MachineContext* context);
  void ReplaceMemberInLoop(const Member& i, const Member& j,
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
