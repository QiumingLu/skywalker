// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/counter.h"
#include "paxos/config.h"

namespace skywalker {

Counter::Counter(const Config* config) : config_(config) {}

void Counter::AddReceivedNode(uint64_t node_id) {
  received_nodes_.insert(node_id);
}

void Counter::AddRejector(uint64_t node_id) { rejectors_.insert(node_id); }

void Counter::AddPromisorOrAcceptor(uint64_t node_id) {
  promisors_or_acceptors_.insert(node_id);
}

bool Counter::IsPassedOnThisRound() const {
  return (promisors_or_acceptors_.size() >= config_->GetMajoritySize());
}

bool Counter::IsRejectedOnThisRound() const {
  return (rejectors_.size() >= config_->GetMajoritySize());
}

bool Counter::IsReceiveAllOnThisRound() const {
  return (received_nodes_.size() >= config_->GetNodeSize());
}

void Counter::StartNewRound() {
  received_nodes_.clear();
  rejectors_.clear();
  promisors_or_acceptors_.clear();
}

}  // namespace skywalker
