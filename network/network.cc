#include "network/network.h"
#include "skywalker/logging.h"
#include "paxos/node_util.h"
#include <string.h>

namespace skywalker {

Network::Network(uint64_t node_id)
    : my_node_id_(node_id),
      bg_loop_(),
      loop_(nullptr),
      server_(nullptr) {
  loop_ = bg_loop_.Loop();
}

Network::~Network() {
  loop_->Exit();
  delete server_;
}

void Network::StartServer(const std::function<void (const Slice&) >& cb) {
  IpPort i;
  ParseNodeId(my_node_id_, &i);
  voyager::SockAddr addr(i.ip, i.port);
  server_ = new voyager::TcpServer(loop_, addr);

  server_->SetMessageCallback(
      [cb](const voyager::TcpConnectionPtr&, voyager::Buffer* buf) {
    while (true) {
      if (buf->ReadableSize() >= kHeaderSize) {
        int size;
        memcpy(&size, buf->Peek(), kHeaderSize);
        if (buf->ReadableSize() >= static_cast<size_t>(size)) {
          cb(Slice(buf->Peek() + kHeaderSize, size - kHeaderSize));
          buf->Retrieve(size);
        } else {
          break;
        }
      } else {
        break;
      }
    }
  });

  server_->Start();
}

void Network::SendMessage(uint64_t node_id,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->QueueInLoop([node_id, content_ptr, this] () {
    std::string s;
    if (SerializeToString(content_ptr, &s)) {
      SendMessageInLoop(node_id, s);
    }
  });
}

void Network::SendMessage(const Membership& m,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->QueueInLoop([this, m, content_ptr] () {
    std::string s;
    if (SerializeToString(content_ptr, &s)) {
      for (int i = 0; i < m.node_id_size(); ++i) {
        if (m.node_id(i) != my_node_id_) {
          SendMessageInLoop(m.node_id(i), s);
        }
      }
    }
  });
}

void Network::SendMessageInLoop(uint64_t node_id,
                                const std::string& s) {
  auto it = connection_map_.find(node_id);
  if (it != connection_map_.end()) {
    it->second->SendMessage(s);
  } else {
    IpPort i;
    ParseNodeId(node_id, &i);
    voyager::SockAddr addr(i.ip, i.port);
    voyager::TcpClient* client(new voyager::TcpClient(loop_, addr));

    client->SetConnectionCallback(
        [node_id, s, this](const voyager::TcpConnectionPtr& p) {
      auto iter = connection_map_.find(node_id);
      if (iter == connection_map_.end()) {
        connection_map_.insert(std::make_pair(node_id, p));
        p->SendMessage(s);
      } else {
        p->ShutDown();
        iter->second->SendMessage(s);
      }
    });

    client->SetConnectFailureCallback([client]() {
      delete client;
    });

    client->SetCloseCallback(
        [node_id, client, this](const voyager::TcpConnectionPtr& p) {
      connection_map_.erase(node_id);
      delete client;
    });

    client->Connect(false);
  }
}

bool Network::SerializeToString(const std::shared_ptr<Content>& content_ptr,
                                std::string* s) {
  char buf[kHeaderSize];
  memset(buf, 0, kHeaderSize);
  s->append(buf, kHeaderSize);
  bool res = content_ptr->AppendToString(s);
  if (res) {
    int size = static_cast<int>(s->size());
    memcpy(buf, &size, kHeaderSize);
    s->replace(s->begin(), s->begin() + kHeaderSize, buf, kHeaderSize);
  } else {
    SWLog(ERROR,
          "Network::SendMessage - Content.SerializeToString error.\n");
  }
  return res;
}

}  // namespace skywalker
