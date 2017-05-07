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

struct IpPort {
  std::string ip;
  uint16_t port;

  IpPort()
      : ip(), port(0) {
  }

  IpPort(const std::string& s, uint16_t n)
      : ip(s), port(n) {
  }
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
  std::vector<IpPort> membership;
  std::vector<IpPort> followers;

  GroupOptions();
};

struct Options {
  IpPort ipport;
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
