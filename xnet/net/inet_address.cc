#include "xnet/net/inet_address.h"

#include <strings.h>
#include "xnet/net/socket_utils.h"

namespace xnet {
namespace net {

const in_addr_t InetAddress::kInAddrAny = INADDR_ANY;
static_assert(sizeof(InetAddress) == sizeof(sockaddr_in), "incorrect InetAddress size");

InetAddress::InetAddress(uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = SocketUtils::HostToNetwork32(kInAddrAny);
  addr_.sin_port = SocketUtils::HostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  SocketUtils::FromHostPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::ToHostPort() const {
  char buf[32];
  SocketUtils::ToHostPort(buf, sizeof buf, addr_);
  return buf;
}

}  // namespace net
}  // namespace xnet