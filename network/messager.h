#ifndef SKYWALKER_NETWORK_MESSAGER_H_
#define SKYWALKER_NETWORK_MESSAGER_H_

#include <string>

#include "network/network.h"
#include "paxos/paxos.pb.h"

namespace skywalker {

class Config;

class Messager {
 public:
  Messager(Config* config, Network* network);

  void SendMessage(uint64_t node_id, Content* content);
  void BroadcastMessage(Content* content);
  void BroadcastMessageToFollower(Content* content);

  Content* PackMessage(ContentType type,
                       PaxosMessage* pmsg,
                       CheckPointMessage* cmsg);

 private:
  Config* config_;
  Network* network_;

  // No copying allowed
  Messager(const Messager&);
  void operator=(const Messager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_NETWORK_MESSAGER_H_
