#ifndef VOYAGER_PAXOS_COUNTER_H_
#define VOYAGER_PAXOS_COUNTER_H_

#include <stdint.h>
#include <set>

namespace voyager {
namespace paxos {

class Config;

class Counter {
 public:
  Counter(const Config* config);

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
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_COUNTER_H_
