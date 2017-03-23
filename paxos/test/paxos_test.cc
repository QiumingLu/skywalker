// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <vector>
#include <voyager/util/string_util.h>
#include "skywalker/node.h"

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
  // options.ipport.ip = "127.0.0.1";
  // options.ipport.port = 5666;
  // options.membership.push_back(options.ipport);

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
    node->Propose(
        0, value, nullptr, [](skywalker::MachineContext*, const skywalker::Status& s, uint64_t instance_id) {
      printf("%s\n", s.ToString().c_str());
    });
  }

  delete node;
  printf("paxos test end\n");

  return 0;
}
