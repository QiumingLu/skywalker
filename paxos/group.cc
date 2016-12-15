#include "paxos/group.h"

namespace skywalker {

Group::Group(uint32_t group_id, uint64_t node_id,
             const Options& options, Network* network)
    : config_(group_id, node_id, options, network),
      instance_(&config_) {
}

bool Group::Start() {
  bool ret = config_.Init();
  if (ret) {
    ret = instance_.Init();
  }
  config_.GetLoop()->SetInstance(&instance_);
  config_.GetLoop()->Loop();
  return ret;
}

int Group::OnReceiveValue(const Slice& value,
                          MachineContext* context,
                          uint64_t* new_instance_id) {
  return instance_.OnReceiveValue(value, context, new_instance_id);
}

void Group::OnReceiveContent(Content* content) {
  instance_.OnReceiveContent(content);
}

}  // namespace skywalker
