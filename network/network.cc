// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "network/network.h"

#include <string.h>
#include <utility>

#include "paxos/config.h"
#include "skywalker/cluster.h"
#include "skywalker/logging.h"
#include "util/coding.h"

namespace skywalker {

Network::Network(const Member& my, uint32_t thread_size)
    : my_(my), thread_size_(thread_size), net_loop_(voyager::kPoll) {
  loop_ = net_loop_.Loop();
}

Network::~Network() {}

void Network::StartServer(const std::function<void(Content&&)>& cb) {
  cb_ = cb;
  voyager::SockAddr addr(my_.host, my_.port);
  server_.reset(new voyager::TcpServer(loop_, addr, "SkywalkerServer", thread_size_));
  server_->SetMessageCallback(
      [this](const voyager::TcpConnectionPtr& p, voyager::Buffer* buf) {
        OnMessage(p, buf);
      });

  server_->Start();
}

void Network::SendMessage(uint64_t node_id, Config* config,
                          const Content& content) {
  std::string s;
  SerializeToString(content, &s);
  loop_->QueueInLoop([this, node_id, config, s = std::move(s)]() {
    auto it = connection_map_.find(node_id);
    if (it != connection_map_.end()) {
      voyager::TcpConnectionPtr p = it->second->GetTcpConnectionPtr();
      if (p) {
        p->SendMessage(std::move(s));
      }
    } else {
      if (config->GetCluster()) {
        auto cluster_map = config->GetCluster()->GetNewestMembership(config->GetGroupId());
        if (cluster_map.find(node_id) != cluster_map.end()) {
          const auto& value = cluster_map[node_id];
          MemberMessage member;
          member.set_id(value.id);
          member.set_host(value.host);
          member.set_port(value.port);
          SendMessageInLoop(member, std::move(s));
        }
      } else {
        std::shared_ptr<Membership> temp = config->GetMembership();
        auto iter = temp->members().find(node_id);
        if (iter != temp->members().end()) {
          SendMessageInLoop(iter->second, std::move(s));
        }
      }
    }
  });
}

void Network::SendMessage(const std::shared_ptr<Membership>& m,
                          const Content& content) {
  std::string s;
  SerializeToString(content, &s);
  loop_->QueueInLoop([this, m, s = std::move(s)]() {
    for (auto& i : m->members()) {
      if (i.first != my_.id) {
        auto it = connection_map_.find(i.first);
        if (it != connection_map_.end()) {
          voyager::TcpConnectionPtr p = it->second->GetTcpConnectionPtr();
          if (p) {
            p->SendMessage(std::move(s));
          }
        } else {
          SendMessageInLoop(i.second, std::move(s));
        }
      }
    }
  });
}

void Network::SendMessageInLoop(const MemberMessage& member,
                                std::string s) {
  uint64_t node_id = member.id();
  voyager::SockAddr addr(member.host(), static_cast<uint16_t>(member.port()));
  std::unique_ptr<voyager::TcpClient> client(
      new voyager::TcpClient(loop_, addr, "SkywalkerClient"));

  client->SetConnectionCallback(
      [s = std::move(s)](const voyager::TcpConnectionPtr& p) {
         p->SendMessage(std::move(s)); 
      });

  client->SetCloseCallback([this, node_id](const voyager::TcpConnectionPtr& p) {
    connection_map_.erase(node_id);
  });

  client->SetConnectFailureCallback(
      [this, node_id]() { connection_map_.erase(node_id); });

  client->Connect(false);
  connection_map_.insert(std::make_pair(node_id, std::move(client)));
}

bool Network::SerializeToString(const Content& content, std::string* s) {
  uint32_t size = kHeaderSize + static_cast<uint32_t>(content.ByteSizeLong());
  s->reserve(size);
  PutFixed32(s, size);
  bool res = content.AppendToString(s);
  if (!res) {
    LOG_ERROR("Network::SendMessage - content serialize to string failed.");
    s->clear();
  }
  return res;
}

void Network::OnMessage(const voyager::TcpConnectionPtr& p,
                        voyager::Buffer* buf) {
  bool res = true;
  while (res) {
    if (buf->ReadableSize() >= kHeaderSize) {
      uint32_t size = DecodeFixed32(buf->Peek());
      if (buf->ReadableSize() >= static_cast<size_t>(size)) {
        Content content;
        res = content.ParseFromArray(buf->Peek() + kHeaderSize, size - kHeaderSize);
        if (res) {
          cb_(std::move(content));
          buf->Retrieve(size);
        } else {
          LOG_ERROR("Network::OnMessage - content parse from array failed.");
          p->ShutDown();
        }
        continue;
      }
    }
    break;
  }
}

}  // namespace skywalker
