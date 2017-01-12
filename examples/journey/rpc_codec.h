#ifndef JOURNEY_RPC_CODEC_H_
#define JOURNEY_RPC_CODEC_H_

#include <google/protobuf/message.h>
#include "voyager/core/buffer.h"

namespace voyager {

class RpcCodec {
 public:
  RpcCodec() { }
  bool ParseFromBuffer(Buffer* buf,
                       google::protobuf::Message* message);
  bool SerializeToString(const google::protobuf::Message& msg,
                         std::string* s);
 private:
  static const int kHeaderSize = 4;

  // No copying allowed
  RpcCodec(const RpcCodec&);
  void operator=(const RpcCodec&);
};

}  // namespace voyager

#endif  // JOURNEY_RPC_CODEC_H_
