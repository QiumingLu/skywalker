// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/node_util.h"

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace skywalker {

uint64_t MakeId(const std::string& ip, uint16_t port) {
  uint64_t id = static_cast<uint64_t>(::inet_addr(ip.c_str()));
  assert(id != (UINTMAX_MAX - 1));
  return (id << 32) | port;
}

void ParseId(uint64_t id, std::string* ip, uint16_t* port) {
  *port = static_cast<uint16_t>(id & (0xffffffff));
  in_addr addr;
  addr.s_addr = static_cast<in_addr_t>(id >> 32);
  *ip = std::string(inet_ntoa(addr));
}

}  // namespace skywalker
