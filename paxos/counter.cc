#include "paxos/counter.h"
#include "paxos/config.h"

namespace skywalker {

Counter::Counter(const Config* config) : config_(config) {
}

void Counter::AddReceivedNode(uint64_t node_id) {
  if (received_nodes_.find(node_id) == received_nodes_.end()) {
    received_nodes_.insert(node_id);
  }
}

void Counter::AddRejector(uint64_t node_id) {
  if (rejectors_.find(node_id) == rejectors_.end()) {
    rejectors_.insert(node_id);
  }
}

void Counter::AddPromisorOrAcceptor(uint64_t node_id) {
  if (promisors_or_acceptors_.find(node_id) == promisors_or_acceptors_.end()) {
    promisors_or_acceptors_.insert(node_id);
  }
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
