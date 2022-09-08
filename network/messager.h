// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_NETWORK_MESSAGER_H_
#define SKYWALKER_NETWORK_MESSAGER_H_

#include <stdint.h>

#include "network/network.h"
#include "proto/paxos.pb.h"

namespace skywalker {

class Config;

class Messager {
 public:
  Messager(Config* config, Network* network);

  void SendMessage(uint64_t node_id, const Content& content);
  void BroadcastMessage(const Content& content);
  void BroadcastMessageForLearn(const Content& content);

 private:
  Config* config_;
  Network* network_;

  // No copying allowed
  Messager(const Messager&);
  void operator=(const Messager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_NETWORK_MESSAGER_H_
