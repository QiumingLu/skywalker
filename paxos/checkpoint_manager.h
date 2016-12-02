#ifndef VOYAGER_PAXOS_CHECKPONIT_MANAGER_H_
#define VOYAGER_PAXOS_CHECKPONIT_MANAGER_H_

namespace voyager {
namespace paxos {

class CheckpointManager {
 public:
  CheckpointManager();

 private:
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_CHECKPONIT_MANAGER_H_
