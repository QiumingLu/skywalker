// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/config.h"
#include "paxos/node_util.h"
#include "skywalker/logging.h"
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
      db_(new DB(this)),
      messager_(new Messager(this, network)),
      machine_manager_(new MachineManager(this)),
      checkpoint_manager_(new CheckpointManager(this)),
      log_manager_(new LogManager(this)),
      membership_machine_(nullptr),
      master_machine_(nullptr) {
  char name[8];
  if (log_storage_path_[log_storage_path_.size() - 1] != '/') {
    snprintf(name, sizeof(name), "/g%u", group_id_);
  } else {
    snprintf(name, sizeof(name), "g%u", group_id_);
  }

  log_storage_path_ += name;
  checkpoint_path_ = log_storage_path_ + "/checkpoint";
  log_path_ = log_storage_path_ + "/log";

  membership_.set_version(0);
  for (auto& i : options.membership) {
    membership_.add_node_id(MakeNodeId(i));
  }
  for (auto& i : options.followers) {
    followers_.add_node_id(MakeNodeId(i));
  }
}

Config::~Config() {
  delete log_manager_;
  delete checkpoint_manager_;
  delete machine_manager_;
  delete messager_;
  delete db_;
}

bool Config::Init() {
  FileManager::Instance()->CreateDir(log_storage_path_);
  FileManager::Instance()->CreateDir(checkpoint_path_);
  bool exists = FileManager::Instance()->FileExists(checkpoint_path_);
  if (!exists) {
    LOG_ERROR("Group %u - checkpoint path(%s) access failed.",
              group_id_, checkpoint_path_.c_str());
    return false;
  }
  int res = db_->Open(log_path_);
  if (res != 0) {
    LOG_ERROR("Group %u - db open failed, which path is %s.",
              group_id_, log_storage_path_.c_str());
    return false;
  }

  for (auto machine : machines_) {
    machine_manager_->AddMachine(machine);
  }
  machine_manager_->AddMachine(membership_machine_);
  if (master_machine_) {
    machine_manager_->AddMachine(master_machine_);
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
