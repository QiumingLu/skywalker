#ifndef VOYAGER_PAXOS_CONFIG_H_
#define VOYAGER_PAXOS_CONFIG_H_

#include <stdint.h>
#include <stddef.h>
#include <set>
#include <vector>

#include "voyager/paxos/nodeinfo.h"
#include "voyager/paxos/options.h"
#include "voyager/paxos/network/messager.h"
#include "voyager/paxos/storage/db.h"
#include "voyager/paxos/state_machine_impl.h"

namespace voyager {
namespace paxos {

class Config {
 public:
  Config(uint32_t group_id, const Options& options, Network* network);
  ~Config();

  bool Init();

  DB* GetDB() const { return db_; }
  Messager* GetMessager() const { return messager_; }
  StateMachineImpl* GetStateMachine() const { return state_machine_; }

  uint32_t GetGroupId() const { return group_id_; }

  bool LogSync() const { return log_sync_; }
  uint32_t SyncInterval() const { return sync_interval_; }

  uint64_t GetNodeId() const { return node_id_; }
  size_t GetNodeSize() const { return membership_.size(); }

  size_t GetMajoritySize() const {
    return (membership_.size() / 2 + 1);
  }

  std::set<uint64_t>& MemberShip() { return membership_; }
  std::vector<NodeInfo>& FollowNodes() { return follow_nodes_; }

  bool IsValidNodeId(uint64_t node_id) const;

 private:
  uint32_t group_id_;
  uint64_t node_id_;

  std::string log_storage_path_;
  bool log_sync_;
  uint32_t sync_interval_;

  std::set<uint64_t> membership_;
  std::vector<NodeInfo> follow_nodes_;

  DB* db_;
  Messager* messager_;
  StateMachineImpl* state_machine_;

  // No copying allowed
  Config(const Config&);
  void operator=(const Config&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_CONFIG_H_
