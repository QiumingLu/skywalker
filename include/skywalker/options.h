#ifndef SKYWALKER_INCLUDE_OPTIONS_H_
#define SKYWALKER_INCLUDE_OPTIONS_H_

#include <string>
#include <vector>

#include "skywalker/nodeinfo.h"

namespace skywalker {

struct Options {
  std::string log_storage_path;
  bool log_sync;
  uint32_t sync_interval;
  uint32_t group_size;
  NodeInfo node_info;
  std::vector<NodeInfo> all_other_nodes;
  std::vector<NodeInfo> follow_nodes;
};

}  // namespace skywalker

#endif   // SKYWALKER_INCLUDE_OPTIONS_H_
