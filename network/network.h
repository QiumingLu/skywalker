// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_NETWORK_NETWORK_H_
#define SKYWALKER_NETWORK_NETWORK_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <voyager/core/bg_eventloop.h>
#include <voyager/core/buffer.h>
#include <voyager/core/eventloop.h>
#include <voyager/core/sockaddr.h>
#include <voyager/core/tcp_client.h>
#include <voyager/core/tcp_server.h>

#include "proto/paxos.pb.h"
#include "skywalker/options.h"
#include "skywalker/slice.h"

namespace skywalker {

class Config;

class Network {
 public:
  explicit Network(const Member& my, uint32_t thread_size = 0);
  ~Network();

  void StartServer(const std::function<void(Content&&)>& cb);

  void SendMessage(uint64_t node_id, Config* config, const Content& content);

  void SendMessage(const std::shared_ptr<Membership>& m,
                   const Content& content);

 private:
  static const uint32_t kHeaderSize = 4;

  void SendMessageInLoop(const MemberMessage& member, std::string s);
  bool SerializeToString(const Content& content, std::string* s);
  void OnMessage(const voyager::TcpConnectionPtr& p, voyager::Buffer* buf);

  Member my_;
  uint32_t thread_size_;
  std::function<void(Content&&)> cb_;
  std::unique_ptr<voyager::TcpServer> server_;
  std::map<uint64_t, std::unique_ptr<voyager::TcpClient> > connection_map_;

  voyager::EventLoop* loop_;
  voyager::BGEventLoop net_loop_;

  // No copying allowed
  Network(const Network&);
  void operator=(const Network&);
};

}  // namespace skywalker

#endif  // SKYWALKER_NETWORK_NETWORK_H_
