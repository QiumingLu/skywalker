// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/config.h"
#include "skywalker/file.h"
#include "skywalker/logging.h"

namespace skywalker {

Config::Config(uint64_t node_id, uint32_t group_id, const GroupOptions& options,
               Network* network)
    : node_id_(node_id),
      group_id_(group_id),
      propose_timeout_(options.propose_timeout),
      log_sync_(options.log_sync),
      sync_interval_(options.sync_interval),
      keep_log_count_(options.keep_log_count),
      log_storage_path_(options.log_storage_path),
      db_(new DB(this)),
      messager_(new Messager(this, network)),
      machine_manager_(new MachineManager(this)),
      checkpoint_manager_(new CheckpointManager(this)),
      log_manager_(new LogManager(this)),
      membership_machine_(new MembershipMachine(this, options)),
      master_machine_(new MasterMachine(this)) {
  char name[8];
  if (log_storage_path_[log_storage_path_.size() - 1] != '/') {
    snprintf(name, sizeof(name), "/g%u", group_id_);
  } else {
    snprintf(name, sizeof(name), "g%u", group_id_);
  }
  
  FileManager::Instance()->CreateDir(log_storage_path_);

  log_storage_path_ += name;
  log_path_ = log_storage_path_ + "/log";
  checkpoint_path_ = log_storage_path_ + "/checkpoint";
  temp_checkpoint_path_ = checkpoint_path_ + "/temp";

  for (auto machine : options.machines) {
    machine_manager_->AddMachine(machine);
  }
  machine_manager_->AddMachine(membership_machine_);
  machine_manager_->AddMachine(master_machine_);
}

Config::~Config() {
  delete master_machine_;
  delete membership_machine_;
  delete log_manager_;
  delete checkpoint_manager_;
  delete machine_manager_;
  delete messager_;
  delete db_;
}

bool Config::Recover() {
  FileManager::Instance()->CreateDir(log_storage_path_);
  FileManager::Instance()->CreateDir(checkpoint_path_);
  FileManager::Instance()->CreateDir(temp_checkpoint_path_);
  if (!FileManager::Instance()->FileExists(checkpoint_path_) ||
      !FileManager::Instance()->FileExists(temp_checkpoint_path_)) {
    LOG_ERROR("Group %u - checkpoint path(%s) temp(%s) access failed.", group_id_,
              checkpoint_path_.c_str(), temp_checkpoint_path_.c_str());
    return false;
  }

  int res = db_->Open(log_path_);
  if (res != 0) {
    LOG_ERROR("Group %u - db open failed, which path is %s.", group_id_,
              log_storage_path_.c_str());
    return false;
  }

  machine_manager_->Recover();
  LOG_INFO("Group %u - Config recover successful!", group_id_);

  return true;
}

bool Config::IsValidNodeId(uint64_t node_id) const {
  std::shared_ptr<Membership> temp = membership_machine_->GetMembership();
  if (temp->members().find(node_id) != temp->members().end()) {
    return true;
  }
  return false;
}

}  // namespace skywalker
