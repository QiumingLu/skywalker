#ifndef VOYAGER_HTTP_HTTP_RESPONSE_PARSER_H_
#define VOYAGER_HTTP_HTTP_RESPONSE_PARSER_H_

#include "voyager/http/http_response.h"

namespace voyager {

class Buffer;

class HttpResponseParser {
 public:
  HttpResponseParser();

  bool ParseBuffer(Buffer* buf);
  bool FinishParse() const { return state_ == kEnd; }

  HttpResponsePtr GetResponse() const { return response_; }

  void Reset();

 private:
  enum ParserState {
    kLine,
    kHeaders,
    kBody,
    kEnd
  };

  bool ParseResponseLine(const char* begin, const char* end);
  bool ParseResponseBody(Buffer* buf);

  ParserState state_;
  HttpResponsePtr response_;

  // No copying allowed
  HttpResponseParser(const HttpResponseParser&);
  void operator=(const HttpResponseParser&);
};

}  // namespace voyager

#endif  // VOYAGER_HTTP_HTTP_RESPONSE_PARSER_H_
