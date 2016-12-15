#ifndef VOYAGER_HTTP_HTTP_RESPONSE_H_
#define VOYAGER_HTTP_HTTP_RESPONSE_H_

#include <memory>
#include <string>

#include "voyager/http/http_message.h"
#include "voyager/core/buffer.h"

namespace voyager {

class HttpResponse : public HttpMessage {
 public:
  HttpResponse()
      : close_(false), status_code_("200"), reason_parse_("OK") {
  }

  void SetCloseState(bool close) { close_ =  close; }
  bool CloseState() const { return close_; }

  void SetStatusCode(const char* begin, const char* end) {
    status_code_.assign(begin, end);
  }
  void SetStatusCode(const std::string& code) { status_code_ = code; }
  const std::string& GetStatusCode() const { return status_code_; }

  void SetReasonParse(const char* begin, const char* end) {
    reason_parse_.assign(begin, end);
  }
  void SetReasonParse(const std::string& s) { reason_parse_ = s; }
  const std::string& ReasonParse() const { return reason_parse_; }

  Buffer& ResponseMessage();

 private:
  bool close_;
  std::string status_code_;
  std::string reason_parse_;
  Buffer message_;
};

typedef std::shared_ptr<HttpResponse> HttpResponsePtr;

}  // namespace voyager

#endif  // VOYAGER_HTTP_HTTP_RESPONSE_H_
