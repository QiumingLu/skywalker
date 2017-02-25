// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_BALLOT_NUMBER_H_
#define SKYWALKER_PAXOS_BALLOT_NUMBER_H_

#include <stdint.h>

namespace skywalker {

class BallotNumber {
 public:
  BallotNumber() : proposal_id_(0), node_id_(0) { }

  BallotNumber(uint64_t proposal_id, uint64_t node_id)
      : proposal_id_(proposal_id), node_id_(node_id) {
  }

  void SetProposalId(uint64_t id) { proposal_id_ = id; }
  uint64_t GetProposalId() const { return proposal_id_; }

  void SetNodeId(uint64_t id) { node_id_ = id; }
  uint64_t GetNodeId() const { return node_id_; }

  void Reset() {
    proposal_id_ = 0;
    node_id_ = 0;
  }

 private:
  uint64_t proposal_id_;
  uint64_t node_id_;
};

inline bool operator==(const BallotNumber& x, const BallotNumber& y) {
  return (x.GetProposalId() == y.GetProposalId()) &&
         (x.GetNodeId() == y.GetNodeId());
}

inline bool operator!=(const BallotNumber& x, const BallotNumber& y) {
  return !(x == y);
}


inline bool operator<(const BallotNumber& x, const BallotNumber& y) {
  if (x.GetProposalId() == y.GetProposalId()) {
    return x.GetNodeId() < y.GetNodeId();
  } else {
    return x.GetProposalId() < y.GetProposalId();
  }
}

inline bool operator>(const BallotNumber& x, const BallotNumber& y) {
  if (x.GetProposalId() == y.GetProposalId()) {
    return x.GetNodeId() > y.GetNodeId();
  } else {
    return x.GetProposalId() > y.GetProposalId();
  }
}

inline bool operator<=(const BallotNumber& x, const BallotNumber& y) {
  return ((x == y) || (x < y));
}

inline bool operator>=(const BallotNumber& x, const BallotNumber& y) {
  return ((x == y) || (x > y));
}

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_BALLOT_NUMBER_H_
