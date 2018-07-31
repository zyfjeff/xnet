#ifndef XNET_NET_SOCKET_H_
#define XNET_NET_SOCKET_H_

#include "xnet/base/macros.h"

namespace xnet {
namespace net {

class InetAddress;

class Socket {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {
  }
  ~Socket();
  int sockfd() const { return sockfd_; }
  void BindAddress(const InetAddress& local_addr);
  void Listen();
  int Accept(InetAddress* peeraddr);
  void SetReuseAddr(bool on);
 private:
  const int sockfd_;
  DISALLOW_COPY_AND_ASSIGN(Socket);
};

}  // namespace net
}  // namespace xnet

#endif // XNET_NET_SOCKET_H_