// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <voyager/util/string_util.h>
#include <voyager/core/tcp_client.h>
#include <voyager/core/sockaddr.h>
#include <voyager/core/eventloop.h>
#include <voyager/rpc/rpc_channel.h>
#include "journey.pb.h"

namespace journey {

class JourneyClient {
 public:
  JourneyClient(voyager::EventLoop* loop, const std::string& server);
  ~JourneyClient();

  void Propose();

 private:
  bool GetLine(bool server = false);
  void Propose(const std::vector<std::string>& v);
  void CreateNewChannel(const RequestMessage& request);
  bool CreateNewRequest(const std::vector<std::string>& v,
                        RequestMessage* request);
  void Done(ResponseMessage* response);
  void HandleError(voyager::ErrorCode code);

  voyager::EventLoop* loop_;
  std::string server_;
  std::vector<std::string> v_;
  std::map<std::string, voyager::RpcChannel*> channels_;

  // No copying allowed
  JourneyClient(const JourneyClient&);
  void operator=(const JourneyClient&);
};

JourneyClient::JourneyClient(voyager::EventLoop* loop,
                             const std::string& server)
    : loop_(loop),
      server_(server) {
}

JourneyClient::~JourneyClient() {
  for (auto& it : channels_) {
    delete it.second;
  }
}

bool JourneyClient::GetLine(bool server) {
  printf("> ");
  std::string s;
  std::getline(std::cin, s);
  if (s == "quit") {
    printf("bye!\n");
    // FIXME
    exit(0);
  } else if (!server) {
    v_.clear();
    size_t found = s.find_first_of(' ');
    if (found != std::string::npos) {
      v_.push_back(s.substr(0, found));
      s.erase(0, found);
      std::vector<std::string> temp;
      voyager::SplitStringUsing(s, ":", &temp);
      for (auto& it : temp) {
        v_.push_back(it);
      }
    } else {
      printf("invalid command!\n");
      GetLine(server);
    }
  } else {
    server_ = s;
  }
  return true;
}

void JourneyClient::Propose() {
  if (GetLine(false)) {
    Propose(v_);
  }
}

void JourneyClient::Propose(const std::vector<std::string>& v) {
  RequestMessage request;
  if (CreateNewRequest(v, &request)) {
    auto it = channels_.find(server_);
    if (it != channels_.end()) {
      ResponseMessage* response = new ResponseMessage();
      JourneyService_Stub stub(it->second);
      stub.Propose(
          nullptr, &request, response,
          google::protobuf::NewCallback(this, &JourneyClient::Done, response));
    } else {
      CreateNewChannel(request);
    }
  } else {
    Propose();
  }
}

void JourneyClient::CreateNewChannel(const RequestMessage& request) {
  std::vector<std::string> ipport;
  voyager::SplitStringUsing(server_, ":", &ipport);
  if (ipport.size() != 2) {
    printf("%s, invalid server_ip:server_port, "
           "please enter new server_ip:server_port\n", server_.c_str());
    if (GetLine(true)) {
      Propose(v_);
    }
    return;
  }

  voyager::SockAddr addr(ipport[0], std::stoi(ipport[1]));
  voyager::TcpClient* client(new voyager::TcpClient(loop_, addr));

  client->SetConnectionCallback(
      [request, this](const voyager::TcpConnectionPtr& p) {
    voyager::RpcChannel* channel = new voyager::RpcChannel(loop_);
    channels_[server_] = channel;
    ResponseMessage* response = new ResponseMessage();
    channel->SetTcpConnectionPtr(p);
    channel->SetErrorCallback(
        std::bind(&JourneyClient::HandleError, this, std::placeholders::_1));
    p->SetMessageCallback(
        std::bind(&voyager::RpcChannel::OnMessage, channel,
                  std::placeholders::_1, std::placeholders::_2));
    JourneyService_Stub stub(channel);
    stub.Propose(
        nullptr, &request, response,
        google::protobuf::NewCallback(this, &JourneyClient::Done, response));
  });

  client->SetConnectFailureCallback([client, this]() {
    delete client;
    printf("connect failed, please enter new server_ip:server_port\n");
    if (GetLine(true)) {
      Propose();
    }
  });

  client->SetCloseCallback(
      [client, this](const voyager::TcpConnectionPtr& p) {
    auto it = channels_.find(server_);
    channels_.erase(it);
    delete it->second;
    delete client;
    printf("%s close, please enter new server_ip:server_port\n",
           server_.c_str());
    if (GetLine(true)) {
      Propose();
    }
  });

  client->Connect(false);
}

bool JourneyClient::CreateNewRequest(const std::vector<std::string>& v,
                                     RequestMessage* request) {
  bool res = true;
  assert(v.size() >= 2);
  request->set_key(v[1]);
  if (v[0] == "put") {
    request->set_type(journey::PROPOSE_TYPE_PUT);
    if (v.size() == 3) {
      request->set_value(v[2]);
    } else {
      res = false;
    }
  } else if (v[0] == "get") {
    request->set_type(journey::PROPOSE_TYPE_GET);
  } else if (v[0] == "delete") {
    request->set_type(journey::PROPOSE_TYPE_DELETE);
  } else {
    res = false;
  }
  if (!res) {
    printf("invalid command!\n");
  }
  return res;
}

void JourneyClient::Done(ResponseMessage* response) {
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
  if (response->result() == PROPOSE_RESULT_SUCCESS &&
      !response->value().empty()) {
    printf(", value:%s", response->value().c_str());
  }

  if (response->result() == PROPOSE_RESULT_NOT_MASTER
      && response->has_master()) {
    server_ = response->master_ip();
    server_ += ":";
    server_ += std::to_string(response->master_port() + 1000);
    printf(", redirect to %s\n", server_.c_str());
    Propose(v_);
  } else {
    printf("\n");
    Propose();
  }
}

void JourneyClient::HandleError(voyager::ErrorCode code) {
  const char* buf;
  switch (code) {
    case voyager::ERROR_CODE_OK:
      buf = "ok";
      break;
    case voyager::ERROR_CODE_TIMEOUT:
      buf = "timeout";
      break;
    case voyager::ERROR_CODE_INVALID_REQUEST:
      buf = "invalid request";
      break;
    case voyager::ERROR_CODE_INVALID_METHOD:
      buf = "invalid method";
      break;
    case voyager::ERROR_CODE_INVALID_SERVICE:
      buf = "invalid service";
      break;
    case voyager::ERROR_CODE_UNKNOWN:
    default:
      buf = "unknown error";
      break;
  }
  printf("%s\n", buf);

  Propose();
}

}  // namespace journey

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s server_ip:server_port\n", argv[0]);
    return -1;
  }
  voyager::EventLoop loop;
  journey::JourneyClient client(&loop, argv[1]);
  client.Propose();
  loop.Loop();
  return 0;
}
