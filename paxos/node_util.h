// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_NODE_UTIL_H_
#define SKYWALKER_PAXOS_NODE_UTIL_H_

#include <stdint.h>

#include "skywalker/options.h"

namespace skywalker {

extern uint64_t MakeNodeId(const IpPort& i);

extern void ParseNodeId(uint64_t node_id, IpPort* i);

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_NODE_UTIL_H_
