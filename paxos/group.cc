#include "paxos/group.h"
#include "skywalker/logging.h"
#include "util/mutexlock.h"

namespace skywalker {

Group::Group(uint32_t group_id, uint64_t node_id,
             const Options& options, Network* network)
    : config_(group_id, node_id, options, network),
      instance_(&config_),
      loop_(config_.GetLoop()),
      mutex_(),
      cond_(&mutex_) {
}

bool Group::Start() {
  bool ret = config_.Init();
  if (ret) {
    ret = instance_.Init();
    if (ret) {
      instance_.SetProposeCompleteCallback(
          std::bind(&Group::ProposeComplete, this, std::placeholders::_1));
    }
  }
  return ret;
}

int Group::OnReceivePropose(const Slice& value,
                            uint64_t* now_instance_id) {
  MutexLock lock(&mutex_);
  propose_end_ = false;
  instance_id_ = 0;
  result_ = -1;
  loop_->QueueInLoop([value, this]() {
    instance_.HandlePropose(value);
  });

  while (!propose_end_) {
    cond_.Wait();
  }

  *now_instance_id = instance_id_;
  return result_;
}

void Group::OnReceiveContent(const std::shared_ptr<Content>& c) {
  loop_->QueueInLoop([c, this]() {
    instance_.HandleContent(c);
  });
}

void Group::ProposeComplete(int result) {
  MutexLock lock(&mutex_);
  if (result == 0) {
    SWLog(INFO, "Group::OnReceivePropose - propose new value success.\n");
  } else if (result == 1) {
    SWLog(INFO,
          "Group::OnReceivePropose - propose new value failed! "
          "Because another value has been chosen.\n");
  } else {
    SWLog(INFO, "Group::OnReceivePropose - propose new value timeout.\n");
  }
  result_ = result;
  instance_id_ = instance_.GetInstanceId();
  propose_end_ = true;
  cond_.Signal();
}

}  // namespace skywalker
