#include "paxos/config.h"
#include "paxos/node_util.h"

namespace skywalker {

Config::Config(uint32_t group_id, uint64_t node_id,
               const Options& options, Network* network)
    : group_id_(group_id),
      node_id_(node_id),
      log_storage_path_(options.log_storage_path),
      log_sync_(options.log_sync),
      sync_interval_(options.sync_interval),
      db_(new DB()),
      messager_(new Messager(this, network)),
      state_machine_(new StateMachineImpl(this)),
      loop_(new RunLoop()) {

  char name[8];
  if (log_storage_path_[log_storage_path_.size() - 1] != '/') {
    snprintf(name, sizeof(name), "/g%d", group_id);
  } else {
    snprintf(name, sizeof(name), "g%d", group_id);
  }

  log_storage_path_ += name;

  for (auto i : options.membership) {
    membership_.insert(MakeNodeId(i));
  }

  for (auto i : options.followers) {
    followers_.insert(MakeNodeId(i));
  }
}

Config::~Config() {
  delete loop_;
  delete state_machine_;
  delete messager_;
  delete db_;
}

bool Config::Init() {
  int ret = db_->Open(group_id_, log_storage_path_);
  if (ret != 0) {
    return false;
  }
  return state_machine_->Init();
}

bool Config::IsValidNodeId(uint64_t node_id) const {
  return true;
}

}  // namespace skywalker
