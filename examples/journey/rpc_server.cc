#include "rpc_server.h"
#include "rpc_codec.h"

namespace voyager {

RpcServer::RpcServer(EventLoop* loop,
                     const SockAddr& addr)
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

void RpcServer::OnMessage(const TcpConnectionPtr& p, Buffer* buf) {
  RpcCodec codec;
  RpcMessage msg;
  bool res = codec.ParseFromBuffer(buf, &msg);
  while(res) {
    OnRequest(p, msg);
    res = codec.ParseFromBuffer(buf, &msg);
  }
}

void RpcServer::OnRequest(const TcpConnectionPtr& p,
                          const RpcMessage& msg) {
  ErrorCode error;
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
        p->SetUserData(new int(msg.id()));
        service->CallMethod(
            method, nullptr, request, response,
            google::protobuf::NewCallback(
                this, &RpcServer::Done, response, p));
        error = OK;
      } else {
        error = INVALID_REQUEST;
      }
      delete request;
    } else {
      error = INVALID_METHOD;
    }
  } else {
    error = INVALID_SERVICE;
  }
  if (error != OK) {
    RpcMessage reply_msg;
    reply_msg.set_id(msg.id());
    reply_msg.set_error(error);
    RpcCodec codec;
    std::string s;
    if (codec.SerializeToString(reply_msg, &s)) {
      p->SendMessage(s);
    }
  }
}

void RpcServer::Done(google::protobuf::Message* response,
                     TcpConnectionPtr p) {
  int* id = reinterpret_cast<int*>(p->UserData());
  RpcMessage msg;
  msg.set_id(*id);
  msg.set_data(response->SerializeAsString());
  RpcCodec codec;
  std::string s;
  if (codec.SerializeToString(msg, &s)) {
    p->SendMessage(s);
  }
  delete id;
  delete response;
}

}  // namespace voyager
