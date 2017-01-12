#ifndef SKYWALKER_PAXOS_GROUP_H_
#define SKYWALKER_PAXOS_GROUP_H_

#include <memory>

#include "paxos/config.h"
#include "paxos/instance.h"
#include "paxos/paxos.pb.h"
#include "skywalker/options.h"
#include "skywalker/slice.h"
#include "skywalker/status.h"
#include "util/mutex.h"
#include "machine/membership_machine.h"
#include "machine/master_machine.h"

namespace skywalker {

class Network;

class Group {
 public:
  Group(uint32_t group_id, uint64_t node_id,
        const Options& options, Network* network);

  bool Start();

  void SyncMembership();
  void SyncMaster();

  Status OnPropose(const Slice& value, int machine_id);
  Status OnPropose(const Slice& value, MachineContext* context,
                   uint64_t* instance_id);

  void OnReceiveContent(const std::shared_ptr<Content>& c);

  Status AddMember(const IpPort& ip);
  Status RemoveMember(const IpPort& ip);
  Status ReplaceMember(const IpPort& new_i, const IpPort& old_i);
  void GetMembership(std::vector<IpPort>* result) const;

  void AddMachine(StateMachine* machine);
  void RemoveMachine(StateMachine* machine);

  void SetMasterLeaseTime(uint64_t micros);
  bool GetMaster(IpPort* i, uint64_t* version) const;
  bool IsMaster() const;
  void RetireMaster();

 private:
  typedef std::function<void ()> Func;

  void SyncMembershipInLoop(MachineContext* context);
  void TryBeMaster();
  void TryBeMasterInLoop(MachineContext* context);
  void AddMemberInLoop(uint64_t node_id, MachineContext* context);
  void RemoveMemberInLoop(uint64_t node_id, MachineContext* context);
  void ReplaceMemberInLoop(uint64_t new_node_id, uint64_t old_node_id,
                           MachineContext* context);
  Status NewPropose(const Func& f, uint64_t* instance_id = nullptr);
  void ProposeComplete(Status&& result, uint64_t instance_id);

  const uint64_t node_id_;
  Config config_;
  Instance instance_;
  RunLoop* loop_;

  RunLoop bg_loop_;
  uint64_t lease_timeout_;
  bool retrie_master_;
  MembershipMachine membership_machine_;
  MasterMachine master_machine_;

  Mutex mutex_;
  Condition cond_;
  bool last_finish_;
  bool propose_end_;
  uint64_t instance_id_;
  Status result_;

  // No copying allowed
  Group(const Group&);
  void operator=(const Group&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_GROUP_H_
