// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_OPTIONS_H_
#define SKYWALKER_INCLUDE_OPTIONS_H_

#include <stdint.h>

#include <functional>
#include <string>
#include <vector>

#include "skywalker/checkpoint.h"
#include "skywalker/state_machine.h"
#include "skywalker/status.h"

namespace skywalker {

struct Member {
  uint64_t id;
  std::string host;
  uint16_t port;
  std::string context;
};

struct GroupOptions {
  // Default: true
  bool use_master;

  // Default: true
  bool log_sync;

  // Default: 10 * 1000 * 1000 microseconds
  uint64_t master_lease_time;

  // Default: 5
  uint32_t sync_interval;

  // Default: 100000
  uint32_t keep_log_count;

  // Default: ""
  std::string log_storage_path;

  // Default: nullptr
  Checkpoint* checkpoint;

  std::vector<StateMachine*> machines;
  std::vector<Member> membership;
  std::vector<Member> followers;

  GroupOptions();
};

struct Options {
  Member my;

  // Default: io_thread_size = (groups.size() + 1) / 2
  // the io_thread_size must be (0, groups.size()]
  // the skywalker's thread_size = 5 + io_thread_size
  uint32_t io_thread_size;

  // the index of group options is group id.
  std::vector<GroupOptions> groups;

  Options();
};

typedef std::function<void(uint64_t instance_id, const Status& s,
                           void* context)>
    ProposeCompleteCallback;

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_OPTIONS_H_
