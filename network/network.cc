#include "network/network.h"
#include "skywalker/logging.h"

namespace skywalker {

Network::Network(const NodeInfo& my)
    : addr_(my.GetIP(), my.GetPort()),
      bg_loop_(),
      loop_(nullptr),
      server_(nullptr) {
}

Network::~Network() {
  delete server_;
}

void Network::StartServer(const std::function<void (const Slice&) >& cb) {
  loop_ = bg_loop_.Loop();
  server_ = new voyager::TcpServer(loop_, addr_);
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

void Network::StopServer() {
  loop_->Exit();
}

void Network::SendMessage(const std::vector<NodeInfo>& nodes,
                          const std::shared_ptr<Content>& content_ptr) {
  loop_->RunInLoop([this, nodes, content_ptr] () {
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

void Network::SendMessageInLoop(const NodeInfo& node,
                                const std::string& s) {
  uint64_t node_id = node.GetNodeId();
  auto it = connection_map_.find(node_id);
  if (it != connection_map_.end()) {
    it->second->SendMessage(s);
  } else {
    voyager::SockAddr addr(node.GetIP(), node.GetPort());
    voyager::TcpClient* client(new voyager::TcpClient(loop_, addr));
    client->SetConnectionCallback(
        [this, node_id, s](const voyager::TcpConnectionPtr& p) {
      connection_map_[node_id] = p;
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
