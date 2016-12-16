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
    size_t size = buf->ReadableSize();
    while (size > sizeof(int)) {
      const char* crlf = buf->FindCRLF();
      if (crlf) {
        size_t len = crlf - buf->Peek();
        cb(Slice(buf->Peek(), len));
        buf->RetrieveUntil(crlf + 2);
        size = buf->ReadableSize();
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
    bool res = content_ptr->SerializeToString(&s);
    if (res) {
      s += "\r\n";
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
    s += "\r\n";
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
