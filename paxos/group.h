// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_GROUP_H_
#define SKYWALKER_PAXOS_GROUP_H_

#include <stdint.h>
#include <map>
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

  bool OnPropose(const std::string& value, int machine_id, void* context,
                 const ProposeCompleteCallback& cb);

  bool OnPropose(const std::string& value, int machine_id, void* context,
                 ProposeCompleteCallback&& cb);

  void OnReceiveContent(const std::shared_ptr<Content>& c);

  bool ChangeMember(const std::map<Member, bool>& value,
                    const ChangeMemberCompleteCallback& cb);
  void GetMembership(std::vector<Member>* result, uint64_t* version) const;

  void SetMasterLeaseTime(uint64_t micros);
  bool GetMaster(Member* i, uint64_t* version) const;
  bool IsMaster() const;
  void RetireMaster();

  void StartGC();
  void StopGC();

 private:
  void SyncMembershipInLoop();
  void TryBeMaster();
  void TryBeMasterInLoop();
  bool NewPropose(ProposeHandler&& f);
  void ProposeComplete(void* context,
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
