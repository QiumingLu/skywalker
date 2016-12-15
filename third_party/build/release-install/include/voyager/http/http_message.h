#ifndef VOYAGER_HTTP_HTTP_MESSAGE_H_
#define VOYAGER_HTTP_HTTP_MESSAGE_H_

#include <map>
#include <string>
#include <utility>

namespace voyager {

class HttpMessage {
 public:
  static const char* kConnection;
  static const char* kContentLength;
  static const char* kHost;

  enum HttpVersion {
    kHttp10,
    kHttp11,
    kHttp20,
  };

  HttpMessage() : version_(kHttp10) { }

  void SetVersion(HttpVersion v) { version_ = v; }
  bool SetVersion(const char* begin, const char* end);
  HttpVersion Version() const { return version_; }
  const char* VersionToString() const;

  void AddHeader(const char* begin, const char* colon, const char* end);
  void AddHeader(const std::string& field, const std::string& value);
  void RemoveHeader(const std::string& field);
  std::string Value(const std::string& field) {
    std::string value;
    std::map<std::string, std::string>::iterator it = header_map_.find(field);
    if (it != header_map_.end()) {
      value = it->second;
    }

    return value;
  }
  const std::map<std::string, std::string>& HeaderMap() const {
    return header_map_;
  }

  void SetBody(const std::string& body) { body_ = body; }
  void SetBody(std::string&& body) { body_ = std::move(body); }
  const std::string& Body() const { return body_; }

 protected:
  void TransferField(std::string* s);

  HttpVersion version_;
  std::map<std::string, std::string> header_map_;
  std::string body_;
};

}  // namespace voyager

#endif  // VOYAGER_HTTP_HTTP_MESSAGE_H_
