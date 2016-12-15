#ifndef VOYAGER_CORE_BASE_SOCKET_H_
#define VOYAGER_CORE_BASE_SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "voyager/util/status.h"

namespace voyager {

class BaseSocket {
 public:
  // RAII create a new socket fd.
  BaseSocket(int domain, bool nonblocking);
  // RAII manage a created socket fd.
  explicit BaseSocket(int socketfd);
  ~BaseSocket();

  int SocketFd() const { return fd_; }

  void ShutDownWrite() const;
  void SetNonBlockAndCloseOnExec(bool on) const;
  void SetReuseAddr(bool on) const;
  void SetReusePort(bool on) const;
  void SetKeepAlive(bool on) const;
  void SetTcpNoDelay(bool on) const;

  Status CheckSocketError() const;

  struct sockaddr_storage PeerSockAddr() const;
  struct sockaddr_storage LocalSockAddr() const;
  int IsSelfConnect() const;

  void SetNoAutoCloseFd() { need_close_ = false; }

 protected:
  const int fd_;

 private:
  bool need_close_;

  // No copying allowed
  BaseSocket(const BaseSocket&);
  void operator=(const BaseSocket&);
};

}  // namespace voyager

#endif  // VOYAGER_CORE_BASE_SOCKET_H_
