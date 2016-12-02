#include "voyager/paxos/group.h"
#include "voyager/util/logging.h"

namespace voyager {
namespace paxos {

Group::Group(uint32_t group_id, const Options& options, Network* network)
    : config_(group_id, options, network),
      instance_(&config_) {
}

bool Group::Start() {
  bool ret = config_.Init();
  if (ret) {
    ret = instance_.Init();
  }
  return ret;
}

bool Group::OnReceiveValue(const Slice& value,
                           MachineContext* context,
                           uint64_t* new_instance_id) {
  return instance_.OnReceiveValue(value, context, new_instance_id);
}

void Group::OnReceiveContent(Content* content) {
  instance_.OnReceiveContent(content);
}

}  // namespace paxos
}  // namespace voyager
