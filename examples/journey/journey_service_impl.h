#ifndef JOURNEY_JOURNEY_SERVICE_IMPL_H_
#define JOURNEY_JOURNEY_SERVICE_IMPL_H_

#include "journey.pb.h"

namespace journey {

class JourneyServiceImpl : public JourneyService {
 public:
  virtual void Put(google::protobuf::RpcController* controller,
                   const journey::RequestMessage* request,
                   journey::ResponseMessage* response,
                   google::protobuf::Closure* done);

  virtual void Delete(google::protobuf::RpcController* controller,
                      const journey::RequestMessage* request,
                      journey::ResponseMessage* response,
                      google::protobuf::Closure* done);

  virtual void Get(google::protobuf::RpcController* controller,
                   const journey::RequestMessage* request,
                   journey::ResponseMessage* response,
                   google::protobuf::Closure* done);

};

}  // namespace journey

#endif  // JOURNEY_JOURNEY_SERVICE_IMPL_H_
