#ifndef VOYAGER_PAXOS_OPTIONS_H_
#define VOYAGER_PAXOS_OPTIONS_H_

#include <string>
#include <vector>

#include "voyager/paxos/nodeinfo.h"

namespace voyager {
namespace paxos {

struct Options {
  std::string log_storage_path;
  bool log_sync;
  uint32_t sync_interval;
  uint32_t group_size;
  NodeInfo node_info;
  std::vector<NodeInfo> all_other_nodes;
  std::vector<NodeInfo> follow_nodes;
};

}  // namespace paxos
}  // namespace voyager

#endif   // VOYAGER_PAXOS_OPTIONS_H_
