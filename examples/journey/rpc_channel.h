#ifndef JOURNEY_RPC_CHANNEL_H_
#define JOURNEY_RPC_CHANNEL_H_

#include <map>
#include <string>
#include <google/protobuf/service.h>
#include <voyager/port/atomic_sequence_num.h>
#include <voyager/port/mutex.h>
#include <voyager/port/mutexlock.h>
#include <voyager/core/tcp_connection.h>
#include <voyager/core/buffer.h>

namespace journey {

class RpcChannel : public google::protobuf::RpcChannel {
 public:
  RpcChannel(const voyager::TcpConnectionPtr& p);
  virtual ~RpcChannel();

  void SetService(const std::map<std::string, google::protobuf::Service*>& s) {
    services_ = s;
  }

  virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done);

  void OnMessage(const voyager::TcpConnectionPtr& p, Buffer* buf);

 private:
  typedef std::pair<google::protobuf::Message*,
                    google::protobuf::Closure*> CallData;
  void OnRpcMessage(const RpcMessage& msg_ptr);
  void Done(google::protobuf::Message* response, int id);

  voyager::TcpConnectionPtr conn_;
  voyager::port::SequenceNumber num_;
  voyager::port::Mutex mutex_;
  std::map<int, CallData> call_map_;
  std::map<std::string, google::protobuf::Service*> services_;
};

}  // namespace journey

#endif  // JOURNEY_RPC_CHANNEL_H_
