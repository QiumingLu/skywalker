#ifndef VOYAGER_HTTP_HTTP_REQUEST_H_
#define VOYAGER_HTTP_HTTP_REQUEST_H_

#include <memory>
#include <string>
#include <utility>

#include "voyager/http/http_message.h"
#include "voyager/core/buffer.h"

namespace voyager {

class HttpRequest : public HttpMessage {
 public:
  enum Method {
    kOptions,
    kHead,
    kGet,
    kPost,
    kPut,
    kDelete,
    kTrace,
    kConnect,
    kPatch,
  };

  HttpRequest();

  void SetMethod(Method method) { method_ = method; }
  bool SetMethod(const char* begin, const char* end);
  Method GetMethod() const { return method_; }
  const char* MethodToString() const;

  void SetPath(const std::string& s) { path_ = s; }
  void SetPath(const char* begin, const char* end) {
    path_.assign(begin, end);
  }
  const std::string& Path() const { return path_; }

  void SetQuery(const char* begin, const char* end) {
    query_.assign(begin, end);
  }
  void SetQuery(const std::string& query) { query_ = query; }
  const std::string& Query() const { return query_; }

  Buffer& RequestMessage();

 private:
  Method method_;
  std::string path_;
  std::string query_;
  Buffer message_;
};

typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

}  // namespace voyager

#endif  // VOYAGER_HTTP_HTTP_REQUEST_H_
