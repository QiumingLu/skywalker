// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/config.h"
#include "paxos/node_util.h"
#include "skywalker/logging.h"
#include "util/timerlist.h"
#include "skywalker/file.h"

namespace skywalker {

Config::Config(uint64_t node_id,
               const GroupOptions& options, Network* network)
    : node_id_(node_id),
      group_id_(options.group_id),
      log_sync_(options.log_sync),
      sync_interval_(options.sync_interval),
      keep_log_count_(options.keep_log_count),
      log_storage_path_(options.log_storage_path),
      machines_(options.machines),
      checkpoint_(options.checkpoint),
      db_(new DB()),
      messager_(new Messager(this, network)),
      membership_machine_(nullptr),
      master_machine_(nullptr) {
 char name[8];
  if (log_storage_path_[log_storage_path_.size() - 1] != '/') {
    snprintf(name, sizeof(name), "/g%d", group_id_);
  } else {
    snprintf(name, sizeof(name), "g%d", group_id_);
  }

  log_storage_path_ += name;
  checkpoint_path_ = log_storage_path_ + "/checkpoint";
  log_path_ = log_storage_path_ + "/log";

  for (auto& i : options.membership) {
    membership_.set_version(0);
    membership_.add_node_id(MakeNodeId(i));
  }
  for (auto& i : options.followers) {
    followers_.add_node_id(MakeNodeId(i));
  }
}

Config::~Config() {
  delete messager_;
  delete db_;
}

bool Config::Init() {
  FileManager::Instance()->CreateDir(log_storage_path_);
  FileManager::Instance()->CreateDir(checkpoint_path_);
  bool res = FileManager::Instance()->FileExists(checkpoint_path_);
  if (!res) {
    LOG_ERROR("Config::Init - checkpoint path %s access failed.",
              checkpoint_path_.c_str());
    return false;
  }
  int ret = db_->Open(group_id_, log_path_);
  if (ret != 0) {
    LOG_ERROR("Config::Init - db open failed, which path is %s.",
              log_storage_path_.c_str());
    return false;
  }
  return true;
}

bool Config::IsValidNodeId(uint64_t node_id) const {
  for (int i = 0; i < membership_.node_id_size(); ++i) {
    if (node_id == membership_.node_id(i)) {
      return true;
    }
  }
  return false;
}

}  // namespace skywalker
