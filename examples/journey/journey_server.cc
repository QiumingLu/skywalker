// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include <skywalker/node.h>
#include <skywalker/node_util.h>
#include <voyager/rpc/rpc_server.h>
#include <voyager/util/string_util.h>

#include "journey_service_impl.h"

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

  skywalker::GroupOptions g_options;
  g_options.use_master = true;
  g_options.log_sync = true;
  g_options.sync_interval = 0;
  g_options.keep_log_count = 1000;
  g_options.log_storage_path = std::string(path);

  skywalker::Options options;

  std::vector<std::string> my;
  voyager::SplitStringUsing(std::string(argv[1]), ":", &my);
  options.my.host = my[0];
  options.my.port = atoi(&*(my[1].begin()));
  options.my.id = skywalker::MakeId(options.my.host, options.my.port);

  std::vector<std::string> others;
  voyager::SplitStringUsing(std::string(argv[2]), ",", &others);
  skywalker::Member member;
  for (std::vector<std::string>::iterator it = others.begin();
       it != others.end(); ++it) {
    size_t found = it->find(":");
    if (found != std::string::npos) {
      member.host = it->substr(0, found);
      member.port = atoi(it->substr(found + 1).c_str());
      member.id = skywalker::MakeId(member.host, member.port);

      g_options.membership.push_back(member);
    }
  }

  for (int i = 0; i < 3; ++i) {
    options.groups.push_back(g_options);
  }

  std::string db_path(path);
  db_path += "/db";
  journey::JourneyServiceImpl service;
  bool res = service.Start(db_path, options);
  std::cout << "ip:" << options.my.host << " port:" << (options.my.port + 1000)
            << std::endl;
  if (res) {
    voyager::EventLoop loop;
    voyager::SockAddr addr(options.my.host, options.my.port + 1000);
    voyager::RpcServer server(&loop, addr, options.groups.size());
    server.RegisterService(&service);
    server.Start();
    loop.Loop();
  }

  return 0;
}
