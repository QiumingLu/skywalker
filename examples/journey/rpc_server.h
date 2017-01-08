#ifndef JOURNEY_RPC_SERVER_H_
#define JOURNEY_RPC_SERVER_H_

#include <map>
#include <string>
#include <voyager/core/eventloop.h>
#include <voyager/core/sockaddr.h>
#include <voyager/core/tcp_connection.h>
#include <voyager/core/tcp_server.h>

namespace journey {

class RpcServer {
 public:
  RpcServer(voyager::EventLoop* loop, const voyager::SockAddr& addr);

  void Start();
  void RegisterService(google::protobuf::Service* service);

 private:
  void OnConnection(const voyager::TcpConnectionPtr& p);
  void OnCloseConnection(const voyager::TcpConnectionPtr& p);

  std::map<std::string, google::protobuf::Service*> services_;
};

}  // namespace journey

#endif  // JOURNEY_RPC_SERVER_H_
