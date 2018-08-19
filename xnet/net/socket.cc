#include "xnet/net/socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <strings.h>

#include "xnet/net/socket_utils.h"
#include "xnet/net/inet_address.h"

 
namespace xnet {
namespace net {

Socket::~Socket() {
  SocketUtils::Close(sockfd_);
}

void Socket::BindAddress(const InetAddress& local_addr) {
  SocketUtils::BindOrDie(sockfd_, &local_addr.GetSocketAddrInet());
;}

void Socket::Listen() {
  SocketUtils::ListenOrDie(sockfd_);
}

int Socket::Accept(InetAddress* peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = SocketUtils::Accept(sockfd_, &addr);
  if (connfd >= 0) {
    peeraddr->SetSocketAddrInet(addr);
  }

  return connfd;
}

void Socket::SetReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, 
               &optval, sizeof optval);
}

void Socket::ShutdownWrite() {
  ::shutdown(sockfd_, SHUT_WR);
}

void Socket::SetTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

}  // namespace net
}  // namespace xnet