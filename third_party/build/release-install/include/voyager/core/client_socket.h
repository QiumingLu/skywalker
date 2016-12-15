#ifndef VOYAGER_CORE_CLIENT_SOCKET_H_
#define VOYAGER_CORE_CLIENT_SOCKET_H_

#include "voyager/core/base_socket.h"

namespace voyager {

class ClientSocket : public BaseSocket {
 public:
  ClientSocket(int domain, bool nonblocking)
      : BaseSocket(domain, nonblocking) {
  }

  explicit ClientSocket(int socketfd)
      : BaseSocket(socketfd) {
  }

  int Connect(const struct sockaddr* sa, socklen_t salen) const;
};

}  // namespace voyager

#endif  // VOYAGER_CORE_CLIENT_SOCKET_H_
