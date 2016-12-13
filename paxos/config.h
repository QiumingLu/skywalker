#ifndef SKYWALKER_PAXOS_CONFIG_H_
#define SKYWALKER_PAXOS_CONFIG_H_

#include <stdint.h>
#include <stddef.h>
#include <set>

#include "network/messager.h"
#include "storage/db.h"
#include "machine/state_machine_impl.h"
#include "skywalker/options.h"
#include "paxos/runloop.h"
#include "paxos/instance.h"

namespace skywalker {

class Config {
 public:
  Config(uint32_t group_id, uint64_t node_id,
         const Options& options, Network* network);
  ~Config();

  bool InitAll(Instance* instance);

  DB* GetDB() const { return db_; }
  Messager* GetMessager() const { return messager_; }
  StateMachineImpl* GetStateMachine() const { return state_machine_; }
  RunLoop* GetLoop() const { return loop_; }

  bool LogSync() const { return log_sync_; }
  uint32_t SyncInterval() const { return sync_interval_; }

  uint32_t GetGroupId() const { return group_id_; }

  uint64_t GetNodeId() const { return node_id_; }
  size_t GetNodeSize() const { return membership_.size(); }

  size_t GetMajoritySize() const {
    return (membership_.size() / 2 + 1);
  }

  std::set<uint64_t>& MemberShip() { return membership_; }
  const std::set<uint64_t>& MemberShip() const { return membership_; }

  const std::set<uint64_t>& Followers() const { return followers_; }

  bool IsValidNodeId(uint64_t node_id) const;

 private:
  uint32_t group_id_;
  uint64_t node_id_;

  std::string log_storage_path_;
  bool log_sync_;
  uint32_t sync_interval_;

  std::set<uint64_t> membership_;
  std::set<uint64_t> followers_;

  DB* db_;
  Messager* messager_;
  StateMachineImpl* state_machine_;
  RunLoop* loop_;

  // No copying allowed
  Config(const Config&);
  void operator=(const Config&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_CONFIG_H_
