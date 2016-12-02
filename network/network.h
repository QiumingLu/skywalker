#ifndef VOYAGER_PAXOS_NETWORK_NETWORK_H_
#define VOYAGER_PAXOS_NETWORK_NETWORK_H_

#include <functional>
#include <map>

#include "voyager/paxos/nodeinfo.h"
#include "voyager/core/bg_eventloop.h"
#include "voyager/core/eventloop.h"
#include "voyager/core/sockaddr.h"
#include "voyager/core/tcp_client.h"
#include "voyager/core/tcp_server.h"
#include "voyager/util/slice.h"

namespace voyager {
namespace paxos {

class Network {
 public:
  Network(const NodeInfo& my);
  ~Network();

  void StartServer(const std::function<void (const Slice& s)>& cb);
  void StopServer();

  void SendMessage(const NodeInfo& other, const Slice& message);

 private:
  void SendMessageInLoop(const SockAddr& addr, std::string* s);

  SockAddr addr_;
  BGEventLoop bg_loop_;
  EventLoop* loop_;
  TcpServer* server_;
  std::map<std::string, TcpConnectionPtr> connection_map_;

  // No copying allowed
  Network(const Network&);
  void operator=(const Network&);
};

}  // namespace paxos
}  // namespace voyager

#endif   // VOYAGER_PAXOS_NETWORK_NETWORK_H_
