#include "xnet/net/socket_utils.h"

#include "glog/logging.h"

namespace xnet {
namespace net {


struct sockaddr_in SocketUtils::GetLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof(localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    LOG(ERROR) << "sockets::getLocalAddr";
  }
  return localaddr;
}

int SocketUtils::CreateNonblockingOrDie() {
  int sockfd = ::socket(AF_INET, 
                        SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 
                        IPPROTO_TCP);
  if (sockfd < 0) {
    LOG(FATAL) << "SocketUtils::CreateNonblockingOrDie";
  }

  return sockfd;
}

void SocketUtils::BindOrDie(int sockfd, const struct sockaddr_in* addr) {
  int ret = ::bind(sockfd, sockaddr_cast(addr), sizeof addr);
  if (ret < 0) {
    LOG(FATAL) << "SocketsUtils::BindOrDie";
  }
}

void SocketUtils::ListenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    LOG(FATAL) << "SocketUtils::ListenOrDie";
  }
}

int  SocketUtils::Accept(int sockfd, struct sockaddr_in* addr) {
  socklen_t addrlen = sizeof *addr;
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), 
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    int saved_errno = errno;
    LOG(FATAL) << "SocketUtils::Accept";

    switch(saved_errno) {
    case EAGAIN:
    case ECONNABORTED:
    case EINTR:
    case EPROTO:
    case EPERM:
    case EMFILE:
      errno = saved_errno;
      break;
    case EBADF:
    case EFAULT:
    case EINVAL:
    case ENFILE:
    case ENOBUFS:
    case ENOMEM:
    case ENOTSOCK:
    case EOPNOTSUPP:
      LOG(FATAL) << "unexpected error of ::accpet " << saved_errno;
      break;
    default:
      LOG(FATAL) << "unknow error of :: accept " << saved_errno;
      break;
    }
  }

  return connfd;
}

void SocketUtils::Close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG(FATAL) << "SocketUtils::close";
  }
}

void SocketUtils::ToHostPort(char* buf, size_t size,
                             const struct sockaddr_in& addr) {
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
  uint16_t port = NetworkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void SocketUtils::FromHostPort(const char* ip, uint16_t port,
                               struct sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = HostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    LOG(FATAL) << "SocketUtils::FromHostPort";
  }
}

int SocketUtils::GetSocketError(int sockfd) {
  int optval;
  socklen_t optlen = sizeof optval;
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

}  // namespace net
}  // namespace xnet
