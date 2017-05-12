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
#include "skywalker/checkpoint.h"
#include "skywalker/status.h"

namespace skywalker {

struct Member {
  uint64_t id;
  std::string ip;
  uint16_t port;
  std::string context;
};

struct GroupOptions {
  uint32_t group_id;
  bool use_master;
  bool log_sync;
  uint32_t sync_interval;
  uint32_t keep_log_count;
  std::string log_storage_path;

  Checkpoint* checkpoint;

  std::vector<StateMachine*> machines;
  std::vector<Member> membership;
  std::vector<Member> followers;

  GroupOptions();
};

struct Options {
  Member my;
  std::vector<GroupOptions> groups;

  Options();
};

typedef std::function<void (MachineContext*,
                            const Status&,
                            uint64_t instance_id)> ProposeCompleteCallback;

typedef std::function<void (const Status&,
                            uint64_t instance_id)> MembershipCompleteCallback;

}  // namespace skywalker

#endif   // SKYWALKER_INCLUDE_OPTIONS_H_
