#ifndef VOYAGER_HTTP_HTTP_REQUEST_PARSER_H_
#define VOYAGER_HTTP_HTTP_REQUEST_PARSER_H_

#include "voyager/http/http_request.h"

namespace voyager {

class Buffer;

class HttpRequestParser {
 public:
  HttpRequestParser();

  bool ParseBuffer(Buffer* buf);
  bool FinishParse() const { return state_ == kEnd; }

  HttpRequestPtr GetRequest() const { return request_; }

  void Reset();

 private:
  enum ParserState {
    kLine,
    kHeaders,
    kBody,
    kEnd
  };

  bool ParseRequestLine(const char* begin, const char* end);
  bool ParseRequestBody(Buffer* buf);

  ParserState state_;
  HttpRequestPtr request_;

  // No copying allowed
  HttpRequestParser(const HttpRequestParser&);
  void operator=(const HttpRequestParser&);
};

}  // namespace voyager

#endif  // VOYAGER_HTTP_HTTP_REQUEST_PARSER_H_
