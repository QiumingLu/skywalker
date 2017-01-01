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
    size_t size = sizeof(uint32_t);
    while (true) {
      if (buf->ReadableSize() > size) {
        uint32_t len;
        memcpy(&len, buf->Peek(), size);
        if (buf->ReadableSize() >= len) {
          cb(Slice(buf->Peek() + size, len));
          buf->Retrieve(len);
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
    size_t size = sizeof(uint32_t);
    char buf[size] = {0};
    std::string s(buf, size);
    bool res = content_ptr->AppendToString(&s);
    if (res) {
      uint32_t len = static_cast<uint32_t>(s.size());
      memcpy(buf, &len, size);
      s.replace(s.begin(), s.begin() + size, buf, size);
      SendMessageInLoop(node_id, s);
    } else {
      SWLog(ERROR,
            "Network::SendMessage - Content.SerializeToString error.\n");
    }
  });
}

void Network::SendMessage(const Membership& m,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->QueueInLoop([this, m, content_ptr] () {
    std::string s;
    bool res = content_ptr->SerializeToString(&s);
    if (res) {
      // FIXME
      s += "\r\n";
      for (int i = 0; i < m.node_id_size(); ++i) {
        if (m.node_id(i) != my_node_id_) {
          SendMessageInLoop(m.node_id(i), s);
        }
      }
    } else {
      SWLog(ERROR,
            "Network::SendMessage - Content.SerializeToString error.\n");
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
      connection_map_.insert(std::make_pair(node_id, p));
      p->SendMessage(s);
    });

    client->SetConnectFailureCallback([client, this]() {
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

}  // namespace skywalker
