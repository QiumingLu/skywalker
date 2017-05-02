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
#include <skywalker/node.h>
#include "checkpoint_impl.h"

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

  skywalker::Checkpoint* checkpoint = new CheckpointImpl();

  skywalker::GroupOptions g_options;
  g_options.group_id = 0;
  g_options.use_master = true;
  g_options.log_sync = true;
  g_options.sync_interval = 0;
  g_options.keep_log_count = 1000;
  g_options.log_storage_path = std::string(path);
  g_options.checkpoint = checkpoint;

  skywalker::Options options;

  std::vector<std::string> my;
  voyager::SplitStringUsing(std::string(argv[1]), ":", &my);
  options.ipport.ip = my[0];
  options.ipport.port = atoi(&*(my[1].begin()));

  std::vector<std::string> others;
  voyager::SplitStringUsing(std::string(argv[2]), ",", &others);
  for (std::vector<std::string>::iterator it = others.begin();
       it != others.end(); ++it) {
    size_t found = it->find(":");
    if (found != std::string::npos) {
      std::string ip = it->substr(0, found);
      uint16_t port = atoi(it->substr(found + 1).c_str());
      g_options.membership.push_back(skywalker::IpPort(ip, port));
    }
  }

  options.groups.push_back(g_options);

  skywalker::Node* node = nullptr;
  bool res = skywalker::Node::Start(options, &node);

  while (res) {
    printf("please enter value:\n");
    std::string value;
    std::getline(std::cin, value);
    node->Propose(
        0, value, nullptr,
        [](skywalker::MachineContext*, const skywalker::Status& s,
           uint64_t instance_id) {
      printf("%s\n", s.ToString().c_str());
    });
  }

  delete checkpoint;
  delete node;

  printf("paxos test end\n");

  return 0;
}
