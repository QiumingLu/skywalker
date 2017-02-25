// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/node_util.h"

#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

namespace skywalker {

uint64_t MakeNodeId(const IpPort& i) {
  uint64_t ip = static_cast<uint64_t>(::inet_addr(i.ip.c_str()));
  assert(ip != (UINTMAX_MAX - 1));
  uint64_t node_id = (ip << 32) | i.port;
  return node_id;
}

void ParseNodeId(uint64_t node_id, IpPort* i) {
  i->port = static_cast<uint16_t>(node_id & (0xffffffff));
  in_addr addr;
  addr.s_addr = static_cast<in_addr_t>(node_id >> 32);
  i->ip = std::string(inet_ntoa(addr));
}

}  // namespace skywalker
