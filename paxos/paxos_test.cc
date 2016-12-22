#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <vector>
#include <voyager/util/string_util.h>
#include <voyager/util/logging.h>
#include "skywalker/status.h"
#include "skywalker/node.h"
#include "skywalker/options.h"

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

  skywalker::Options options;
  options.log_storage_path = std::string(path);
  options.log_sync = true;
  options.sync_interval = 0;
  options.group_size = 1;

  std::vector<std::string> my;
  voyager::SplitStringUsing(std::string(argv[1]), ":", &my);
  options.ipport.ip = my[0];
  options.ipport.port = atoi(&*(my[1].begin()));

  std::vector<std::string> others;
  voyager::SplitStringUsing(std::string(argv[2]), ",", &others);
  for (std::vector<std::string>::iterator it = others.begin();
       it != others.end(); ++it) {
    std::vector<std::string> other;
    voyager::SplitStringUsing(*it, ":", &other);
    options.membership.push_back(
        skywalker::IpPort(other[0], atoi(&*(other[1].begin()))));
  }

  skywalker::Node* node = nullptr;
  bool res = skywalker::Node::Start(options, &node);

  while (res) {
    printf("please enter value:\n");
    std::string value;
    std::getline(std::cin, value);
    uint64_t instance_id(0);
    skywalker::Status s = node->Propose(0, value, &instance_id);
    printf("%s\n", s.ToString().c_str());
  }

  printf("paxos test end\n");

  return 0;
}
