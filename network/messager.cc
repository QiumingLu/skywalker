// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "network/messager.h"
#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

Messager::Messager(Config* config, Network* network)
    : config_(config),
      network_(network) {
}

std::shared_ptr<Content> Messager::PackMessage(PaxosMessage* msg) {
  std::shared_ptr<Content> content_ptr(new Content());
  content_ptr->set_type(PAXOS_MESSAGE);
  content_ptr->set_group_id(config_->GetGroupId());
  content_ptr->set_allocated_paxos_msg(msg);
  return content_ptr;
}

std::shared_ptr<Content> Messager::PackMessage(CheckpointMessage* msg) {
  std::shared_ptr<Content> content_ptr(new Content());
  content_ptr->set_type(CHECKPOINT_MESSAGE);
  content_ptr->set_group_id(config_->GetGroupId());
  content_ptr->set_allocated_checkpoint_msg(msg);
  return content_ptr;
}

void Messager::SendMessage(uint64_t node_id,
                           const std::shared_ptr<Content>& content_ptr) {
  assert(node_id != 0);
  assert(node_id != config_->GetNodeId());
  network_->SendMessage(node_id, config_, content_ptr);
}

void Messager::BroadcastMessage(
    const std::shared_ptr<Content>& content_ptr) {
  std::shared_ptr<Membership> temp = config_->GetMembership();
  if (temp->members().size() > 1) {
    network_->SendMessage(temp, content_ptr);
  }
}


void Messager::BroadcastMessageToFollower(
    const std::shared_ptr<Content>& content_ptr) {
  std::shared_ptr<Membership> temp = config_->GetFollowers();
  if (temp->members().size() > 1) {
    network_->SendMessage(temp, content_ptr);
  }
}

}  // namespace skywalker
