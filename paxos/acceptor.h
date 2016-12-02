#ifndef VOYAGER_PAXOS_ACCEPTOR_H_
#define VOYAGER_PAXOS_ACCEPTOR_H_

#include <string>

#include "voyager/paxos/ballot_number.h"
#include "voyager/paxos/paxos.pb.h"

namespace voyager {
namespace paxos {

class Config;
class Instance;
class Messager;

class Acceptor {
 public:
  Acceptor(Config* config, Instance* instance);

  bool Init();

  void SetInstanceId(uint64_t id) { instance_id_ = id; }
  uint64_t GetInstanceId() const { return instance_id_; }

  const BallotNumber& GetPromisedBallot() const { return promised_ballot_; }
  const BallotNumber& GetAcceptedBallot() const { return accepted_ballot_; }

  void OnPrepare(const PaxosMessage& msg);
  void OnAccpet(const PaxosMessage& msg);

  const std::string& GetAcceptedValue() const { return accepted_value_; }

  void NextInstance();

 private:
  int ReadFromDB(uint64_t* instance_id);
  int WriteToDB(uint64_t instance_id, uint32_t last_checksum);

  uint32_t log_sync_count_;

  Config* config_;
  Instance* instance_;
  Messager* messager_;

  BallotNumber promised_ballot_;
  BallotNumber accepted_ballot_;

  uint64_t instance_id_;
  std::string accepted_value_;

  // No copying allowed
  Acceptor(const Acceptor&);
  void operator=(const Acceptor&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_ACCEPTOR_H_
