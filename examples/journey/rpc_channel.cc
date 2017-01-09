#include "rpc_channel.h"
#include "rpc_codec.h"

namespace journey {

RpcChannel::RpcChannel() {
}

RpcChannel::~RpcChannel() {
  for(auto it : call_map_) {
    delete it.second.response;
    delete it.second.done;
  }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
  int id = num_.GetNext();
  RpcMessage msg;
  msg.set_id(id);
  msg.set_service_name(method->service()->full_name());
  msg.set_method_name(method->name());
  msg.set_data(request->SerializeAsString());

  RpcCodec codec;
  std::string s;
  if (codec.SerializeToString(msg, &s)) {
    conn_->SendMessage(s);
    voyager::port::MutexLock lock(&mutex_);
    call_map_[id] = CallData(response, done);
  }
}

void RpcChannel::OnMessage(const voyager::TcpConnectionPtr& p,
                           voyager::Buffer* buf) {
  RpcCodec codec;
  RpcMessage msg;
  bool res = codec.ParseFromBuffer(buf, &msg);
  while (res) {
    OnResponse(msg);
    res = codec.ParseFromBuffer(buf, &msg);
  }
}

void RpcChannel::OnResponse(const RpcMessage& msg) {
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
    data.response->ParseFromString(msg.data());
    if (data.done) {
      data.done->Run();
    }
  }
  delete data.response;
}

}  // namespace journey
