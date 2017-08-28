// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "network/network.h"

#include <string.h>
#include <utility>

#include <voyager/util/coding.h>

#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

Network::Network(const Member& my) : my_(my), net_loop_(voyager::kPoll) {
  loop_ = net_loop_.Loop();
}

Network::~Network() {}

void Network::StartServer(
    const std::function<void(const std::shared_ptr<Content>&)>& cb) {
  cb_ = cb;
  voyager::SockAddr addr(my_.host, my_.port);
  server_.reset(new voyager::TcpServer(loop_, addr, "SkywalkerServer"));
  server_->SetMessageCallback(
      [this](const voyager::TcpConnectionPtr& p, voyager::Buffer* buf) {
        OnMessage(p, buf);
      });

  server_->Start();
}

void Network::SendMessage(uint64_t node_id, Config* config,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->QueueInLoop([this, node_id, config, content_ptr]() {
    std::string s;
    if (SerializeToString(content_ptr, &s)) {
      auto it = connection_map_.find(node_id);
      if (it != connection_map_.end()) {
        voyager::TcpConnectionPtr p = it->second->GetTcpConnectionPtr();
        if (p) {
          p->SendMessage(s);
        } else {
          std::shared_ptr<Membership> temp = config->GetMembership();
          if (temp->members().find(node_id) == temp->members().end()) {
            it->second->Close();
            connection_map_.erase(it);
          }
        }
      } else {
        std::shared_ptr<Membership> temp = config->GetMembership();
        auto iter = temp->members().find(node_id);
        if (iter != temp->members().end()) {
          SendMessageInLoop(iter->second, s);
        }
      }
    }
  });
}

void Network::SendMessage(const std::shared_ptr<Membership>& m,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->QueueInLoop([this, m, content_ptr]() {
    std::string s;
    if (SerializeToString(content_ptr, &s)) {
      for (auto& i : m->members()) {
        if (i.first != my_.id) {
          auto it = connection_map_.find(i.first);
          if (it != connection_map_.end()) {
            voyager::TcpConnectionPtr p = it->second->GetTcpConnectionPtr();
            if (p) {
              p->SendMessage(s);
            }
          } else {
            SendMessageInLoop(i.second, s);
          }
        }
      }
    }
  });
}

void Network::SendMessageInLoop(const MemberMessage& member,
                                const std::string& s) {
  uint64_t node_id = member.id();
  voyager::SockAddr addr(member.host(), static_cast<uint16_t>(member.port()));
  std::unique_ptr<voyager::TcpClient> client(
      new voyager::TcpClient(loop_, addr, "SkywalkerClient"));

  client->SetConnectionCallback(
      [s](const voyager::TcpConnectionPtr& p) { p->SendMessage(s); });

  client->SetCloseCallback([this, node_id](const voyager::TcpConnectionPtr& p) {
    connection_map_.erase(node_id);
  });

  client->SetConnectFailureCallback(
      [this, node_id]() { connection_map_.erase(node_id); });

  client->Connect(true);
  connection_map_.insert(std::make_pair(node_id, std::move(client)));
}

bool Network::SerializeToString(const std::shared_ptr<Content>& content_ptr,
                                std::string* s) {
  char buf[kHeaderLen];
  memset(buf, 0, kHeaderLen);
  s->append(buf, kHeaderLen);
  bool res = content_ptr->AppendToString(s);
  if (res) {
    int32_t size = static_cast<int32_t>(s->size());
    voyager::EncodeFixed32(buf, size);
    s->replace(s->begin(), s->begin() + kHeaderLen, buf, kHeaderLen);
  } else {
    LOG_ERROR("Network::SendMessage - Content.SerializeToString error.");
  }
  return res;
}

void Network::OnMessage(const voyager::TcpConnectionPtr& p,
                        voyager::Buffer* buf) {
  bool res = true;
  while (res) {
    if (buf->ReadableSize() >= kHeaderLen) {
      uint32_t size = voyager::DecodeFixed32(buf->Peek());
      if (buf->ReadableSize() >= static_cast<size_t>(size)) {
        std::shared_ptr<Content> c(new Content());
        res = c->ParseFromArray(buf->Peek() + kHeaderLen, size - kHeaderLen);
        if (res) {
          cb_(c);
          buf->Retrieve(size);
        } else {
          p->ShutDown();
        }
        continue;
      }
    }
    break;
  }
}

}  // namespace skywalker
