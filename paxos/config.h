#ifndef SKYWALKER_PAXOS_CONFIG_H_
#define SKYWALKER_PAXOS_CONFIG_H_

#include <stdint.h>
#include "network/messager.h"
#include "storage/db.h"
#include "skywalker/options.h"
#include "paxos/runloop.h"
#include "util/mutex.h"

namespace skywalker {

class Config {
 public:
  Config(uint32_t group_id, uint64_t node_id,
         const Options& options, Network* network);
  ~Config();

  bool Init();

  DB* GetDB() const { return db_; }
  Messager* GetMessager() const { return messager_; }
  RunLoop* GetLoop() const { return loop_; }
  RunLoop* GetBGLoop() const { return bg_loop_; }

  bool LogSync() const { return log_sync_; }
  uint32_t SyncInterval() const { return sync_interval_; }

  uint32_t GetGroupId() const { return group_id_; }

  uint64_t GetNodeId() const { return node_id_; }
  size_t GetNodeSize() const { return membership_.node_id_size(); }

  size_t GetMajoritySize() const {
    return (membership_.node_id_size() / 2 + 1);
  }

  void SetMembership(const Membership& m) { membership_ = m; }
  const Membership& GetMembership() const { return membership_; }
  const Membership& GetFollowers() const { return followers_; }
  bool IsValidNodeId(uint64_t node_id) const;

 private:
  uint32_t group_id_;
  uint64_t node_id_;

  std::string log_storage_path_;
  bool log_sync_;
  uint32_t sync_interval_;

  DB* db_;
  Messager* messager_;
  RunLoop* loop_;
  RunLoop* bg_loop_;

  Membership membership_;
  Membership followers_;

  // No copying allowed
  Config(const Config&);
  void operator=(const Config&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_CONFIG_H_
