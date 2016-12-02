#ifndef VOYAGER_PAXOS_PROPOSER_H_
#define VOYAGER_PAXOS_PROPOSER_H_

#include <string>

#include "voyager/paxos/ballot_number.h"
#include "voyager/paxos/counter.h"
#include "voyager/paxos/paxos.pb.h"
#include "voyager/util/slice.h"

namespace voyager {
namespace paxos {

class Config;
class Instance;
class Messager;

class Proposer {
 public:
  Proposer(Config* config, Instance* instance);

  void SetInstanceId(uint64_t id) { instance_id_ = id; }
  uint64_t GetInstanceId() const { return instance_id_; }

  void SetStartProposalId(uint64_t id) { proposal_id_ = id; }

  void SetNoSkipPrepare() { skip_prepare_ = false; }

  void NewValue(const Slice& value);

  void OnPrepareReply(const PaxosMessage& msg);
  void OnAccpetReply(const PaxosMessage& msg);

  void NextInstance();

 private:
  void Prepare(bool need_new_ballot = true);
  void Accept();
  void NewChosenValue();

  Config* config_;
  Instance* instance_;
  Messager* messager_;

  BallotNumber hightest_ballot_;
  uint64_t hightest_proprosal_id_;

  uint64_t instance_id_;
  uint64_t proposal_id_;
  std::string value_;

  Counter counter_;

  bool preparing_;
  bool accepting_;
  bool skip_prepare_;
  bool was_rejected_by_someone_;

  // No copying allowed
  Proposer(const Proposer&);
  void operator=(const Proposer&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_PROPOSER_H_
