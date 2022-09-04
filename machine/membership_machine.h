// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
#define SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_

#include <mutex>
#include <string>
#include <vector>

#include "proto/paxos.pb.h"
#include "skywalker/options.h"
#include "skywalker/state_machine.h"

namespace skywalker {

class Config;

class MembershipMachine : public StateMachine {
 public:
  MembershipMachine(Config* config, const GroupOptions& options);

  void SetNewMembershipCallback(const NewMembershipCallback& cb) { cb_ = cb; }

  std::shared_ptr<Membership> GetMembership() const;

  virtual bool Recover(uint32_t group_id, uint64_t instance_id,
                       const std::string& dir,
                       const std::vector<std::string>& files);

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value, void* /* context */);

  virtual bool MakeCheckpoint(uint32_t group_id, uint64_t instance_id,
                              const std::string& dir);

 private:
  Config* config_;

  mutable std::mutex mutex_;
  std::shared_ptr<Membership> membership_;

  NewMembershipCallback cb_;

  // No copying allowed
  MembershipMachine(const MembershipMachine&);
  void operator=(const MembershipMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
