#ifndef VOYAGER_CORE_SOCKADDR_H_
#define VOYAGER_CORE_SOCKADDR_H_

#include <netdb.h>

#include <string>

#include "voyager/util/status.h"

namespace voyager {

class SockAddr {
 public:
  explicit SockAddr(uint16_t port);
  SockAddr(const std::string& host, uint16_t port);

  const struct sockaddr* GetSockAddr() const {
    return reinterpret_cast<const struct sockaddr*>(&sa_);
  }
  sa_family_t Family() const { return sa_.ss_family; }
  std::string Ipbuf() const { return ipbuf_; }

  static int FormatAddress(const char* ip, uint16_t port,
                           char* buf, size_t buf_size);
  static void SockAddrToIP(const struct sockaddr* sa,
                           char* ipbuf, size_t ipbuf_size);
  static int SockAddrToIPPort(const struct sockaddr* sa,
                              char* buf, size_t buf_size);
  static void IPPortToSockAddr(const char* ip, uint16_t port,
                               struct sockaddr_in* sa4);
  static void IPPortToSockAddr(const char* ip, uint16_t port,
                               struct sockaddr_in6* sa6);

  static struct sockaddr_storage LocalSockAddr(int socketfd);
  static struct sockaddr_storage PeerSockAddr(int socketfd);
  static int FormatLocal(int socketfd, char* buf, size_t buf_size);
  static int FormatPeer(int socketfd, char* buf, size_t buf_size);

 private:
  Status GetAddrInfo(const char* host, uint16_t port);

  struct sockaddr_storage sa_;
  std::string ipbuf_;
};

}  // namespace voyager

#endif  // VOYAGER_CORE_SOCKADDR_H_
