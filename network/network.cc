#include "network/network.h"
#include "skywalker/logging.h"
#include "paxos/node_util.h"

namespace skywalker {

Network::Network()
    : bg_loop_(),
      loop_(nullptr),
      server_(nullptr) {
  loop_ = bg_loop_.Loop();
}

Network::~Network() {
  loop_->Exit();
  delete server_;
}

void Network::StartServer(const IpPort& i,
                          const std::function<void (const Slice&) >& cb) {
  voyager::SockAddr addr(i.ip, i.port);
  server_ = new voyager::TcpServer(loop_, addr);

  server_->SetMessageCallback(
      [cb](const voyager::TcpConnectionPtr&, voyager::Buffer* buf) {
    Slice s(buf->Peek(), buf->ReadableSize());
    if (!s.empty() && s.size() > sizeof(int)) {
      cb(s);
      buf->RetrieveAll();
    }
  });

  server_->Start();
}

void Network::SendMessage(uint64_t node_id,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->QueueInLoop([this, node_id, content_ptr] () {
    std::string s;
    bool res = content_ptr->SerializeToString(&s);
    if (res) {
      SendMessageInLoop(node_id, s);
    } else {
      SWLog(ERROR,
            "Network::SendMessage - Content.SerializeToString error.\n");
    }
  });
}

void Network::SendMessage(const std::set<uint64_t>& nodes,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->QueueInLoop([this, nodes, content_ptr] () {
    std::string s;
    bool res = content_ptr->SerializeToString(&s);
    if (res) {
      for (auto n : nodes) {
        SendMessageInLoop(n, s);
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
    IpPort i(ParseNodeId(node_id));
    voyager::SockAddr addr(i.ip, i.port);
    voyager::TcpClient* client(new voyager::TcpClient(loop_, addr));

    client->SetConnectionCallback(
        [this, node_id, s](const voyager::TcpConnectionPtr& p) {
      connection_map_.insert(std::make_pair(node_id, p));
      p->SendMessage(s);
    });

    client->SetConnectFailureCallback([this, client]() {
      delete client;
    });

    client->SetCloseCallback(
        [this, node_id, client](const voyager::TcpConnectionPtr& p) {
      connection_map_.erase(node_id);
      delete client;
    });

    client->Connect(false);
  }
}

}  // namespace skywalker
