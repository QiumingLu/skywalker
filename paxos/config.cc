#include "voyager/paxos/config.h"

namespace voyager {
namespace paxos {

Config::Config(uint32_t group_id, const Options& options, Network* network)
    : group_id_(group_id),
      node_id_(options.node_info.GetNodeId()),
      log_sync_(options.log_sync),
      sync_interval_(options.sync_interval),
      follow_nodes_(options.follow_nodes),
      db_(new DB()),
      messager_(new Messager(this, network)),
      state_machine_(new StateMachineImpl(this)) {

  std::string temp(options.log_storage_path);
  if (temp[temp.size() - 1] != '/') {
    temp += '/';
  }
  char name[512];
  snprintf(name, sizeof(name), "%sg%d", temp.c_str(), group_id);
  log_storage_path_ = std::string(name);

  membership_.insert(node_id_);
  for (auto node : options.all_other_nodes) {
    membership_.insert(node.GetNodeId());
  }
}

Config::~Config() {
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

}  // namespace paxos
}  // namespace voyager
