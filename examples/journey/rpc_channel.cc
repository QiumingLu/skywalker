#include "rpc_channel.h"

namespace journey {

RpcChannel::RpcChannel(const voyager::TcpConnectionPtr& p)
    : conn_(p) {
}

RpcChannel::~RpcChannel() {
  for(auto it : call_map_) {
    delete it->second.response;
    delete it->second.done;
  }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
  RpcMessage msg;
  msg.set_type(REQUEST);
  int id = msg.set_id(num_.GetNext());
  msg.set_service_name(method->service()->full_name());
  msg.set_method_name(method->name());
  msg.set_request_data(request->SerializeAsString());

  size_t size = sizeof(uint32_t);
  char buf[size] = {0};
  std::string s(buf, size);
  bool res = msg.AppendToString(&s);
  if (res) {
    uint32_t len = static_cast<uint32_t>(s.size());
    memcpy(buf, &len, size);
    s.replace(s.begin(), s.begin() + size, buf, size);
    conn_->SendMessage(s);
    voyager::port::MutexLock lock(&mutex_);
    call_map_[id] = std::make_pair(response, done);
  }
}

void RpcChannel::OnMessage(const voyager::TcpConnectionPtr& p, Buffer* buf) {
  size_t size = sizeof(uint32_t);
  while (true) {
    if (buf->ReadableSize() > size) {
      uint32_t len;
      memcpy(&len, buf->Peek(), size);
      if (buf->ReadableSize() >= len) {
        RpcMessage msg;
        msg.ParseFromArray(buf->Peek() + size, len-size);
        OnRpcMessage(msg);
        buf->Retrieve(len);
      } else {
        break;
      }
    } else {
      break;
    }
  }
}

void RpcChannel::OnRpcMessage(const RpcMessage& msg) {
  if (msg.type() == REQUEST) {
    auto it = services_->find(msg.service_name());
    if (it != services_.end()) {
      google::protobuf::Service* service = it->second;
      const google::protobuf::ServiceDescriptor* desc =
          service->GetDescriptor();
      const google::protobuf::MethodDescriptor* method =
          desc->FindMethodByName(msg.method_name());
      if (method) {
        google::protobuf::Message* request(
            service->GetRequestPrototype(method).New());
        if (request->ParseFromString(msg.request_data())) {
          google::protobuf::Message* response =
              service->GetResponsePrototype(method).New();
          service->CallMethod(
              method, nullptr, request, response,
              NewCallback(this, &RpcChannel::Done, response, msg.id()));
        }
        delete request;
      }
    }
  } else if (msg.type() == RESPONSE) {
    int id = msg.id();
    CallData data;
    {
      voyager::port::MutexLock lock(&mutex_);
      auto it = call_map_.find(id);
      if (it != call_map_.end()) {
        data = it->second;
        call_map_.erase(it);
      }
    }
    if (data.response) {
      if (msg.has_response_data()) {
        data.response->ParseFromString(msg.response_data());
      }
      if (data.done) {
        data.done->Run();
      }
    }
    delete data.response;
  }
}

void RpcChannel::Done(google::protobuf::Message* response, int id) {
  RpcMessage msg;
  msg.set_type(RESPONSE);
  msg.set_id(id);
  msg.set_response_data(response->SerializeAsString());

  size_t size = sizeof(uint32_t);
  char buf[size] = {0};
  std::string s(buf, size);
  bool res = msg.AppendToString(&s);
  if (res) {
    uint32_t len = static_cast<uint32_t>(s.size());
    memcpy(buf, &len, size);
    s.replace(s.begin(), s.begin() + size, buf, size);
    conn_->SendMessage(s);
  }
  delete response;
}

}  // namespace journey
