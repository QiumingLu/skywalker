// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "network/messager.h"

#include <memory>

#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

Messager::Messager(Config* config, Network* network)
    : config_(config), network_(network) {}

void Messager::SendMessage(uint64_t node_id, const Content& content) {
  assert(node_id != 0);
  assert(node_id != config_->GetNodeId());
  network_->SendMessage(node_id, config_, content);
}

void Messager::BroadcastMessage(const Content& content) {
  std::shared_ptr<Membership> temp = config_->GetMembership();
  if (temp->members().size() > 0) {
    network_->SendMessage(temp, content);
  }
}

}  // namespace skywalker
