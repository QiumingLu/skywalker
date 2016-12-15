#ifndef VOYAGER_HTTP_HTTP_SERVER_H_
#define VOYAGER_HTTP_HTTP_SERVER_H_

#include <functional>
#include <string>
#include <utility>

#include "voyager/http/http_request.h"
#include "voyager/core/tcp_server.h"

namespace voyager {

class Buffer;
class EventLoop;
class HttpResponse;
class SockAddr;

class HttpServer {
 public:
  typedef std::function<void (HttpRequestPtr, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* ev,
             const SockAddr& addr,
             const std::string& name = std::string("HttpServer"),
             int thread_size = 4);

  void Start();

  void SetHttpCallback(const HttpCallback& cb) { http_cb_ = cb; }
  void SetHttpCallback(HttpCallback&& cb) { http_cb_ = std::move(cb); }

 private:
  void NewConnection(const TcpConnectionPtr& ptr);
  void HandleClose(const TcpConnectionPtr& ptr);
  void HandleMessage(const TcpConnectionPtr& ptr, Buffer* buf);

  HttpCallback http_cb_;
  TcpServer server_;

  HttpServer(const HttpServer&);
  void operator=(const HttpServer&);
};

}  // namespace voyager

#endif  // VOYAGER_HTTP_HTTP_SERVER_H_
