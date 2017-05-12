// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
#define SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_

#include <string>
#include <vector>

#include "proto/paxos.pb.h"
#include "skywalker/state_machine.h"
#include "skywalker/options.h"
#include "util/mutex.h"

namespace skywalker {

class Config;

class MembershipMachine : public StateMachine {
 public:
  MembershipMachine(Config* config, const GroupOptions& options);

  void Recover();

  std::shared_ptr<Membership> GetMembership() const;
  bool HasSyncMembership() const;

  std::string GetString() const;
  void SetString(const std::string& s);

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value,
                       MachineContext* /* context */);

 private:
  Config* config_;
  bool has_sync_membership_;

  mutable Mutex mutex_;
  std::shared_ptr<Membership> membership_;

  // No copying allowed
  MembershipMachine(const MembershipMachine&);
  void operator=(const MembershipMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
