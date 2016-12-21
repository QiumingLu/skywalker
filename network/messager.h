#ifndef SKYWALKER_NETWORK_MESSAGER_H_
#define SKYWALKER_NETWORK_MESSAGER_H_

#include <stdint.h>
#include <memory>

#include "network/network.h"
#include "paxos/paxos.pb.h"

namespace skywalker {

class Config;

class Messager {
 public:
  Messager(Config* config, Network* network);

  void SendMessage(uint64_t node_id,
                   const std::shared_ptr<Content>& content_ptr);
  void BroadcastMessage(const std::shared_ptr<Content>& content_ptr);
  void BroadcastMessageToFollower(
      const std::shared_ptr<Content>& content_ptr);

  std::shared_ptr<Content> PackMessage(PaxosMessage* msg);
  std::shared_ptr<Content> PackMessage(CheckPointMessage* msg);

 private:
  Config* config_;
  Network* network_;

  // No copying allowed
  Messager(const Messager&);
  void operator=(const Messager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_NETWORK_MESSAGER_H_
