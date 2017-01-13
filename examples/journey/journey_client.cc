#include <voyager/util/string_util.h>
#include <voyager/core/tcp_client.h>
#include <voyager/core/sockaddr.h>
#include <voyager/core/eventloop.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "rpc_channel.h"
#include "journey.pb.h"

namespace journey {

class JourneyClient {
 public:
  JourneyClient(voyager::EventLoop* loop);
  ~JourneyClient();

  void Propose(const std::string& s);

 private:
  void CreateNewChannel(const std::vector<std::string>& v,
                        const std::string& key);

  RequestMessage* CreateNewRequest(const std::vector<std::string>& v);

  void Done(RequestMessage* request, ResponseMessage* response);

  voyager::EventLoop* loop_;
  std::map<std::string, voyager::RpcChannel*> channels_;

  // No copying allowed
  JourneyClient(const JourneyClient&);
  void operator=(const JourneyClient&);
};

JourneyClient::JourneyClient(voyager::EventLoop* loop)
    : loop_(loop) {
}

JourneyClient::~JourneyClient() {
  for (auto it : channels_) {
    delete it.second;
  }
}

void JourneyClient::Propose(const std::string& s) {
  std::vector<std::string> v;
  voyager::SplitStringUsing(s, ",", &v);
  std::string key(v[0]);
  key += ":";
  key += v[1];
  auto it = channels_.find(key);
  if (it != channels_.end()) {
    RequestMessage *request = CreateNewRequest(v);
    ResponseMessage* response = new ResponseMessage();
    JourneyService_Stub stub(it->second);
    stub.Propose(
        nullptr, request, response,
        google::protobuf::NewCallback(
            this, &JourneyClient::Done, request, response));
  } else {
    CreateNewChannel(v, key);
  }
}

void JourneyClient::CreateNewChannel(const std::vector<std::string>& v,
                                     const std::string& key) {
  voyager::SockAddr addr(v[0], atoi(&*(v[1].begin())));
  voyager::TcpClient* client(new voyager::TcpClient(loop_, addr));
  RequestMessage *request = CreateNewRequest(v);

  client->SetConnectionCallback(
      [key, request, this](const voyager::TcpConnectionPtr& p) {
    voyager::RpcChannel *channel = new voyager::RpcChannel();
    channels_[key] = channel;
    ResponseMessage* response = new ResponseMessage();
    channel->SetTcpConnectionPtr(p);
    p->SetMessageCallback(
        std::bind(&voyager::RpcChannel::OnMessage, channel,
                  std::placeholders::_1, std::placeholders::_2));
    JourneyService_Stub stub(channel);
    stub.Propose(
        nullptr, request, response,
        google::protobuf::NewCallback(
            this, &JourneyClient::Done, request, response));
  });

  client->SetConnectFailureCallback([client, request]() {
    delete client;
    delete request;
  });

  client->SetCloseCallback(
      [key, client, this](const voyager::TcpConnectionPtr& p) {
    auto it = channels_.find(key);
    channels_.erase(it);
    delete it->second;
    delete client;
  });

  client->Connect(false);
}

RequestMessage* JourneyClient::CreateNewRequest(
    const std::vector<std::string>& v) {
  RequestMessage* request = new RequestMessage();
  request->set_key(v[3]);
  if (v[2] == "put") {
    request->set_type(journey::PROPOSE_TYPE_PUT);
    request->set_value(v[4]);
  } else if (v[2] == "get") {
    request->set_type(journey::PROPOSE_TYPE_GET);
  } else {
    request->set_type(journey::PROPOSE_TYPE_DELETE);
  }
  return request;
}

void JourneyClient::Done(RequestMessage* request,
                         ResponseMessage* response) {
  const char* result;
  switch (response->result()) {
    case PROPOSE_RESULT_SUCCESS:
      result = "success";
      break;
    case PROPOSE_RESULT_FAIL:
      result = "failed";
      break;
    case PROPOSE_RESULT_NOT_FOUND:
      result = "not found";
      break;
    case PROPOSE_RESULT_NOT_MASTER:
      result = "not master";
      break;
    default:
      result = "unknown";
      break;
  }
  printf("%s", result);
  if (request->type() == PROPOSE_TYPE_GET
      && response->result() == PROPOSE_RESULT_SUCCESS) {
    printf(", value:%s", response->value().c_str());
  }

  if (response->result() == PROPOSE_RESULT_NOT_MASTER) {
    printf(", has_master:%d master_ip:%s, master_port:%d",
           response->has_master(), response->master_ip().c_str(),
           response->master_port() + 1000);
  }
  delete request;

  printf("\n\n> ");
  std::string s;
  std::getline(std::cin, s);
  this->Propose(s);
}

}  // namespace journey

int main(int argc, char** argv) {
  printf("Usage: serverip,server_port,operator,key,value\n");
  printf("> ");
  std::string s;
  std::getline(std::cin, s);

  voyager::EventLoop loop;
  journey::JourneyClient client(&loop);
  client.Propose(s);
  loop.Loop();
  return 0;
}
