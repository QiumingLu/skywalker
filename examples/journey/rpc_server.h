#ifndef JOURNEY_RPC_SERVER_H_
#define JOURNEY_RPC_SERVER_H_

#include <map>
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <voyager/core/eventloop.h>
#include <voyager/core/sockaddr.h>
#include <voyager/core/tcp_connection.h>
#include <voyager/core/tcp_server.h>

#include "rpc.pb.h"

namespace journey {

class RpcServer {
 public:
  RpcServer(voyager::EventLoop* loop, const voyager::SockAddr& addr);

  void Start();
  void RegisterService(google::protobuf::Service* service);

 private:
  void OnMessage(const voyager::TcpConnectionPtr& p, voyager::Buffer* buf);
  void OnRequest(const voyager::TcpConnectionPtr& p, const RpcMessage& msg);
  void Done(voyager::TcpConnectionPtr p, int id);

  voyager::TcpServer server_;
  std::map<std::string, google::protobuf::Service*> services_;
};

}  // namespace journey

#endif  // JOURNEY_RPC_SERVER_H_
