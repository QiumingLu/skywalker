#include "rpc_server.h"
#include "rpc_channel.h"

namespace journey {

RpcServer::RpcServer(voyager::EventLoop* loop, const SockAddr& addr)
    : server_(loop, addr) {
  server_.SetConnectionCallback(
      std::bind(&RpcServer::OnConnection, this, std::placesholders::_1));
  server_.SetCloseCallback(
      std::bind(&RpcServer::OnCloseConnection, this, std::placesholders::_1));
}

void RpcServer::Start() {
  server_.Start();
}

void RpcServer::RegisterService(google::protobuf::Service* service) {
  const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
  services_[desc->full_name()] = service;
}

void RpcServer::OnConnection(const voyager::TcpConnectionPtr& p) {
  RpcChannel* channel(new RpcChannel(p));
  channel->SetService(services_);
  p->SetUserData(channel);
  p->SetMessageCallback(
      std::bind(&RpcChannel::OnMessage, channel,
                std::placesholders::_1, std::placesholders::_2));
}

void RpcServer::OnCloseConnection(const voyager::TcpConnectionPtr& p) {
  delete reinterpret_cast<RpcChannel*>(p->UserData());
}

}  // namespace journey
