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
  explicit MembershipMachine(Config* config);

  void Recover();

  void SetMembership(const Membership& m);
  const Membership& GetMembership() const;
  void GetMembership(std::vector<IpPort>* result) const;
  bool HasSyncMembership() const;

  std::string GetString() const;
  void SetString(const std::string& s);

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value,
                       MachineContext* /* context */);
 private:
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
