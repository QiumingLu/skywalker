#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <vector>
#include "voyager/util/string_util.h"
#include "voyager/paxos/node.h"
#include "voyager/paxos/options.h"
#include "voyager/paxos/nodeinfo.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: %s myip:myport node0_ip:node0_port,...\n", argv[0]);
    return -1;
  }

  char path[1024];
  if (getcwd(path, sizeof(path)) == nullptr) {
    printf("getcwd error\n");
    return -1;
  }

  voyager::paxos::Options options;
  options.log_storage_path = std::string(path);
  options.log_sync = true;
  options.sync_interval = 1;
  options.group_size = 1;

  std::vector<std::string> my;
  voyager::SplitStringUsing(std::string(argv[1]), ":", &my);
  voyager::paxos::NodeInfo my_node_info(my[0], atoi(&*(my[1].begin())));
  options.node_info = my_node_info;

  std::vector<std::string> others;
  voyager::SplitStringUsing(std::string(argv[2]), ",", &others);
  for (std::vector<std::string>::iterator it = others.begin();
       it != others.end(); ++it) {
    std::vector<std::string> other;
    voyager::SplitStringUsing(*it, ":", &other);
    voyager::paxos::NodeInfo other_node_info(other[0],
                                             atoi(&*(other[1].begin())));
    options.all_other_nodes.push_back(other_node_info);
  }

  voyager::paxos::Node* node = nullptr;
  bool res = voyager::paxos::Node::Start(options, &node);

  std::string value;
  while (res) {
    printf("please enter value:\n");
    getline(std::cin, value);
    uint64_t instance_id(0);
    bool success = node->Propose(0, value, &instance_id);
    printf("bool success:%d, it's instance_id is %" PRIu64"\n",
           success, instance_id);
  }

  printf("paxos test end\n");

  return 0;
}
