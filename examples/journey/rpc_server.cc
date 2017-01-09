#include "rpc_server.h"
#include "rpc_codec.h"

namespace journey {

RpcServer::RpcServer(voyager::EventLoop* loop,
                     const voyager::SockAddr& addr)
    : server_(loop, addr) {
  server_.SetMessageCallback(
      std::bind(&RpcServer::OnMessage, this,
                std::placeholders::_1, std::placeholders::_2));
}

void RpcServer::Start() {
  server_.Start();
}

void RpcServer::RegisterService(google::protobuf::Service* service) {
  const google::protobuf::ServiceDescriptor* desc =
      service->GetDescriptor();
  assert(desc != nullptr);
  services_[desc->full_name()] = service;
}

void RpcServer::OnMessage(const voyager::TcpConnectionPtr& p,
                          voyager::Buffer* buf) {
  RpcCodec codec;
  RpcMessage msg;
  bool res = codec.ParseFromBuffer(buf, &msg);
  while(res) {
    OnRequest(p, msg);
    res = codec.ParseFromBuffer(buf, &msg);
  }
}

void RpcServer::OnRequest(const voyager::TcpConnectionPtr& p,
                          const RpcMessage& msg) {
  auto it = services_.find(msg.service_name());
  if (it != services_.end()) {
    google::protobuf::Service* service = it->second;
    const google::protobuf::ServiceDescriptor* desc =
        service->GetDescriptor();
    const google::protobuf::MethodDescriptor* method =
        desc->FindMethodByName(msg.method_name());
    if (method) {
      google::protobuf::Message* request(
          service->GetRequestPrototype(method).New());
      if (request->ParseFromString(msg.data())) {
        google::protobuf::Message* response =
            service->GetResponsePrototype(method).New();
        service->CallMethod(
            method, nullptr, request, response,
   //         NewCallback(std::bind(&RpcServer::Done, this, p, response, msg.id())));
            NewCallback(this, &RpcServer::Done, p, response, msg.id()));
      }
      delete request;
    }
  }
}

void RpcServer::Done(const voyager::TcpConnectionPtr& p,
                     google::protobuf::Message* response, int id) {
  RpcMessage msg;
  msg.set_id(id);
  msg.set_data(response->SerializeAsString());
  RpcCodec codec;
  std::string s;
  if (codec.SerializeToString(msg, &s)) {
    p->SendMessage(s);
  }
  delete response;
}

}  // namespace journey
