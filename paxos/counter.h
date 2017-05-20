// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_COUNTER_H_
#define SKYWALKER_PAXOS_COUNTER_H_

#include <stdint.h>
#include <set>

namespace skywalker {

class Config;

class Counter {
 public:
  explicit Counter(const Config* config);

  void AddReceivedNode(uint64_t node_id);
  void AddRejector(uint64_t node_id);
  void AddPromisorOrAcceptor(uint64_t node_id);

  bool IsPassedOnThisRound() const;
  bool IsRejectedOnThisRound() const;
  bool IsReceiveAllOnThisRound() const;

  void StartNewRound();

 private:
  const Config* config_;

  std::set<uint64_t> received_nodes_;
  std::set<uint64_t> rejectors_;
  std::set<uint64_t> promisors_or_acceptors_;

  // Intentionally copyable
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_COUNTER_H_
