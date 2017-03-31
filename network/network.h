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
#include <voyager/core/eventloop.h>
#include <voyager/core/sockaddr.h>
#include <voyager/core/tcp_client.h>
#include <voyager/core/tcp_server.h>
#include <voyager/core/buffer.h>

#include "skywalker/slice.h"
#include "skywalker/options.h"
#include "proto/paxos.pb.h"

namespace skywalker {

class Network {
 public:
  explicit Network(uint64_t node_id);
  ~Network();

  void StartServer(const std::function<void (const Slice& s)>& cb);

  void SendMessage(uint64_t node_id,
                   const std::shared_ptr<Content>& content_ptr);

  void SendMessage(const Membership& m,
                   const std::shared_ptr<Content>& content_ptr);

 private:
  static const int kHeaderSize = 4;

  bool SerializeToString(const std::shared_ptr<Content>& content_ptr,
                         std::string* s);
  void SendMessageInLoop(uint64_t node_id, const std::string& s);

  uint64_t my_node_id_;
  std::unique_ptr<voyager::TcpServer> server_;
  std::map<uint64_t, voyager::TcpConnectionPtr> connection_map_;

  voyager::EventLoop* loop_;
  voyager::BGEventLoop net_loop_;

  // No copying allowed
  Network(const Network&);
  void operator=(const Network&);
};

}  // namespace skywalker

#endif   // SKYWALKER_NETWORK_NETWORK_H_
