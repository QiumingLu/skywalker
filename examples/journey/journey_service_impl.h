#ifndef JOURNEY_JOURNEY_SERVICE_IMPL_H_
#define JOURNEY_JOURNEY_SERVICE_IMPL_H_

#include <skywalker/node.h>
#include "journey.pb.h"
#include "journey_db_machine.h"

namespace journey {

class JourneyServiceImpl : public JourneyService {
 public:
  JourneyServiceImpl();
  ~JourneyServiceImpl();

  bool Start(const std::string& db_path, const skywalker::Options& options);

  virtual void Propose(google::protobuf::RpcController* controller,
                       const journey::RequestMessage* request,
                       journey::ResponseMessage* response,
                       google::protobuf::Closure* done);

 private:
  uint32_t Shard(const std::string& key);

  JourneyDBMachine machine_;
  uint32_t group_size_;
  skywalker::Node* node_;

  // No copying allowed
  JourneyServiceImpl(const JourneyServiceImpl&);
  void operator=(const JourneyServiceImpl&);
};

}  // namespace journey

#endif  // JOURNEY_JOURNEY_SERVICE_IMPL_H_
