// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
#define SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_

#include <string>
#include <vector>

#include "skywalker/state_machine.h"
#include "skywalker/options.h"
#include "storage/db.h"
#include "util/mutex.h"

namespace skywalker {

class Config;

class MembershipMachine : public StateMachine {
 public:
  explicit MembershipMachine(const Options& options, Config* config);

  void Recover();

  const Membership& GetMembership() const;
  void GetMembership(std::vector<IpPort>* result) const;
  bool HasSyncMembership() const;

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value,
                       MachineContext* /* context */);

  virtual uint64_t GetCheckpointInstanceId(uint32_t group_id) const;

 public:
  Config* config_;
  DB* db_;

  mutable Mutex mutex_;
  bool has_sync_membership_;
  Membership membership_;

  // No copying allowed
  MembershipMachine(const MembershipMachine&);
  void operator=(const MembershipMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
