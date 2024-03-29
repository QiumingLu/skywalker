// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_OPTIONS_H_
#define SKYWALKER_INCLUDE_OPTIONS_H_

#include <stdint.h>

#include <functional>
#include <string>
#include <vector>

#include "skywalker/state_machine.h"
#include "skywalker/status.h"

namespace skywalker {

class Cluster;

typedef std::function<void(uint32_t group_id)> NewMembershipCallback;

typedef std::function<void(uint32_t group_id)> NewMasterCallback;

typedef std::function<void(uint64_t instance_id, const Status& s,
                           void* context)>
    ProposeCompleteCallback;

struct Member {
  uint64_t id;
  std::string host;
  uint16_t port;
  std::string context;
};

struct GroupOptions {
  // Default: true
  bool use_master;

  // Default: 10 * 1000 ms
  uint64_t master_lease_time;

  // Default: 1000 ms
  uint64_t propose_timeout;

  // Default: 3
  uint32_t keep_checkpoint_count;

  // Default: true
  bool log_sync;

  // Default: 5
  uint32_t sync_interval;

  // Default: 100000
  uint32_t keep_log_count;

  // Default: ""
  std::string log_storage_path;

  std::vector<StateMachine*> machines;

  // must be the same as the first time
  std::vector<Member> membership;

  GroupOptions();
};

struct Options {
  // The skywalker's thread model is:
  //  ______________________________________________
  // | name                 |        size           |
  // |————————————————————————————————————————————--|
  // | network thread       |        N + 1          |
  // |                                              |
  // | master thread        |          N            |
  // |                                              |
  // | learn thread         |          N            |
  // |                                              |
  // | clean thread         |          N            |
  // |                                              |
  // | io thread            |          N            |
  // |                                              |
  // | callback thread      |          N            |
  // |                                              |
  // | leveldb thread model |         0/1           |
  //  -----------------------------------------------
  // The skywalker's thread size is:
  // 2 + net_thread_size + io_thread_size + callback_thread_size +
  // learn_thread_size + clean_thread_size + master_thread_size

  // Default: net_thread_size = 1
  uint32_t net_thread_size;

  // Default: io_thread_size = (groups.size() + 1) / 2
  // the io_thread_size must be (0, groups.size()]
  uint32_t io_thread_size;

  // Default: 1
  // the callback_thread_size must be (0, groups.size()]
  uint32_t callback_thread_size;

  // Default: 1
  // the learn_thread_size must be (0, groups.size()]
  uint32_t learn_thread_size;

  // Default: 1
  // the clean_thread_size must be (0, groups.size()]
  uint32_t clean_thread_size;

  // Default: 1
  // the master_thread_size must be [0, groups.size()]
  uint32_t master_thread_size;

  Member my;

  // the index of group options is group id.
  std::vector<GroupOptions> groups;

  // The node may be not initialize completely when this callback.
  NewMembershipCallback membership_cb;

  // The node may be not initialize completely when this callback.
  NewMasterCallback master_cb;

  // The node will call it to get the cluster information.
  Cluster* cluster;

  Options();
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_OPTIONS_H_
