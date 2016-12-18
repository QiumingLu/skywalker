#ifndef SKYWALKER_PAXOS_CHECKPONIT_MANAGER_H_
#define SKYWALKER_PAXOS_CHECKPONIT_MANAGER_H_

namespace skywalker {

class CheckpointManager {
 public:
  CheckpointManager();

 private:
  // No copying allowed
  CheckpointManager(const CheckpointManager&);
  void operator=(const CheckpointManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_CHECKPONIT_MANAGER_H_
