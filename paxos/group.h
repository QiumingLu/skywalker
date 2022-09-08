// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_GROUP_H_
#define SKYWALKER_PAXOS_GROUP_H_

#include <stdint.h>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "machine/master_machine.h"
#include "machine/membership_machine.h"
#include "paxos/config.h"
#include "paxos/instance.h"
#include "paxos/propose_queue.h"
#include "paxos/threadpool.h"
#include "proto/paxos.pb.h"
#include "skywalker/options.h"

namespace skywalker {

class Network;

class Group {
 public:
  Group(uint64_t node_id, uint32_t group_id, const GroupOptions& options,
        Network* network, Cluster* cluster);
  ~Group();

  bool Recover();
  void Start(RunLoop* io_loop, RunLoop* callback_loop,
             RunLoop* learn_loop, RunLoop* clean_loop, RunLoop* master_loop);

  void SetNewMembershipCallback(const NewMembershipCallback& cb);
  void SetNewMasterCallback(const NewMasterCallback& cb);

  void Sync();

  bool OnPropose(uint32_t machine_id, const std::string& value, void* context,
                 const ProposeCompleteCallback& cb);

  bool OnPropose(uint32_t machine_id, const std::string& value, void* context,
                 ProposeCompleteCallback&& cb);

  void OnContent(Content&& content);

  bool ChangeMember(const std::vector<std::pair<Member, bool>>& value,
                    void* context, const ProposeCompleteCallback& cb);
  void GetMembership(std::vector<Member>* result, uint64_t* version) const;

  bool GetMaster(Member* i, uint64_t* version) const;
  bool IsMaster() const;
  void RetireMaster();

  void StartGC();
  void StopGC();

 private:
  void TryBeMaster();
  void TryBeMasterInLoop();
  bool NewPropose(ProposeHandler&& f);
  void ProposeComplete(uint64_t instance_id, const Status& result,
                       void* context);

  const uint64_t node_id_;
  Config config_;
  Instance instance_;

  bool use_master_;
  bool retrie_master_;
  uint64_t lease_timeout_;
  uint64_t now_;
  MembershipMachine* membership_machine_;
  MasterMachine* master_machine_;
  ProposeCompleteCallback propose_cb_;
  TimerId timer_;

  std::mutex mutex_;
  std::condition_variable cond_;
  bool propose_end_;
  Status result_;

  ProposeQueue propose_queue_;
  RunLoop* io_loop_;
  RunLoop* master_loop_;

  // No copying allowed
  Group(const Group&);
  void operator=(const Group&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_GROUP_H_
