#ifndef SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
#define SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_

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

  Membership GetMembership() const;
  void GetMembership(std::vector<IpPort>* result) const;
  bool IsValidNodeId(uint64_t node_id) const;
  bool HasSyncMembership() const;

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value);

 public:
  Config* config_;
  DB* db_;

  mutable Mutex mutex_;
  bool has_sync_membership_;
  Membership membership_;

  // No cpying allowed
  MembershipMachine(const MembershipMachine&);
  void operator=(const MembershipMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_MEMBERSHIP_MACHINE_H_
