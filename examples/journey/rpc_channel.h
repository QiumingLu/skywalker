#ifndef JOURNEY_RPC_CHANNEL_H_
#define JOURNEY_RPC_CHANNEL_H_

#include <functional>
#include <map>
#include <google/protobuf/service.h>
#include "voyager/port/atomic_sequence_num.h"
#include "voyager/port/mutex.h"
#include "voyager/port/mutexlock.h"
#include "voyager/core/tcp_connection.h"
#include "voyager/core/buffer.h"
#include "voyager/core/eventloop.h"
#include "rpc.pb.h"
#include "rpc_codec.h"

namespace voyager {

class RpcChannel : public google::protobuf::RpcChannel {
 public:
  typedef std::function<void (ErrorCode code)> ErrorCallback;

  RpcChannel(EventLoop* loop);
  virtual ~RpcChannel();

  void SetTcpConnectionPtr(const TcpConnectionPtr& p) {
    conn_ = p;
  }

  void SetTimeout(uint64_t micros) {
    micros_ = micros;
  }

  void SetErrorCallback(const ErrorCallback& cb) {
    error_cb_ = cb;
  }

  virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done);

  void OnMessage(const TcpConnectionPtr& p, Buffer* buf);

 private:
  struct CallData {
    google::protobuf::Message* response;
    google::protobuf::Closure* done;
    TimerId timer;

    CallData()
        : response(nullptr), done(nullptr), timer() {
    }
    CallData(google::protobuf::Message* r,
             google::protobuf::Closure* d,
             TimerId t)
        : response(r), done(d), timer(t) {
    }
  };

  void TimeoutHandler(int id);
  void OnResponse(const RpcMessage& msg);

  EventLoop* loop_;
  uint64_t micros_;
  RpcCodec codec_;
  TcpConnectionPtr conn_;
  ErrorCallback error_cb_;
  port::SequenceNumber num_;
  port::Mutex mutex_;
  std::map<int, CallData> call_map_;

  // No copying allowed
  RpcChannel(const RpcChannel&);
  void operator=(const RpcChannel&);
};

}  // namespace voyager

#endif  // JOURNEY_RPC_CHANNEL_H_
