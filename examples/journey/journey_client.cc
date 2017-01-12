#include <voyager/core/tcp_client.h>
#include <voyager/core/sockaddr.h>
#include <voyager/core/eventloop.h>
#include <stdio.h>
#include <iostream>
#include "rpc_channel.h"
#include "journey.pb.h"

void Done(journey::ResponseMessage* response) {
  std::cout << "result:" << response->result() << " "
            << "value:" << response->value() << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 6) {
    printf("Usage: %s serverip server_port operator key value\n", argv[0]);
    return -1;
  }

  journey::RequestMessage* request = new journey::RequestMessage();
  if (memcmp(argv[3], "put", 3) == 0) {
    request->set_type(journey::PROPOSE_TYPE_PUT);
  } else if (memcmp(argv[3], "get", 3) == 0) {
    request->set_type(journey::PROPOSE_TYPE_GET);
  } else {
    request->set_type(journey::PROPOSE_TYPE_DELETE);
  }
  request->set_key(argv[4]);
  request->set_value(argv[5]);

  voyager::SockAddr addr(argv[1], atoi(argv[2]));
  voyager::EventLoop loop;
  voyager::TcpClient client(&loop, addr);
  voyager::RpcChannel *channel = new voyager::RpcChannel();
  client.SetConnectionCallback(
      [request, channel](const voyager::TcpConnectionPtr& p) {
    journey::ResponseMessage* response = new journey::ResponseMessage();
    channel->SetTcpConnectionPtr(p);
    journey::JourneyService_Stub stub(channel);
    stub.Propose(
        nullptr, request, response,
        google::protobuf::NewCallback(&Done, response));
    delete request;
  });
  client.SetMessageCallback(
      [channel](const voyager::TcpConnectionPtr& p, voyager::Buffer* buf) {
    channel->OnMessage(p, buf);
  });

  client.Connect();
  loop.Loop();
  delete channel;
  return 0;
}
