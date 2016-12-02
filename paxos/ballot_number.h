#ifndef VOYAGER_PAXOS_BALLOT_NUMBER_H_
#define VOYAGER_PAXOS_BALLOT_NUMBER_H_

#include <stdint.h>

namespace voyager {
namespace paxos {

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

  bool operator>=(const BallotNumber& other) const {
    if (proposal_id_ == other.proposal_id_) {
      return node_id_ >= other.node_id_;
    } else {
      return proposal_id_ > other.proposal_id_;
    }
  }

  bool operator!=(const BallotNumber& other) const {
    return (proposal_id_ != other.proposal_id_) || (node_id_ != other.node_id_);
  }

  bool operator==(const BallotNumber& other) const {
    return (proposal_id_ == other.proposal_id_) && (node_id_ == other.node_id_);
  }

  bool operator>(const BallotNumber& other) const {
    if (proposal_id_ == other.proposal_id_) {
      return node_id_ > other.node_id_;
    } else {
      return proposal_id_ > other.proposal_id_;
    }
  }

  void Reset() {
    proposal_id_ = 0;
    node_id_ = 0;
  }

 private:
  uint64_t proposal_id_;
  uint64_t node_id_;
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_BALLOT_NUMBER_H_
