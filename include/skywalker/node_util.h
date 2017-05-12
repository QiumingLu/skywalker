// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_SKYWALKER_NODE_UTIL_H_
#define SKYWALKER_INCLUDE_SKYWALKER_NODE_UTIL_H_

#include <stdint.h>
#include <string>

namespace skywalker {

extern uint64_t MakeId(const std::string& ip, uint16_t port);

extern void ParseId(uint64_t id, std::string* ip, uint16_t* port);

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_SKYWALKER_NODE_UTIL_H_
