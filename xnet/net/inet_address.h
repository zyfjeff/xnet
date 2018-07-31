#ifndef XNET_NET_INET_ADDRESS_H_
#define XNET_NET_INET_ADDRESS_H_

#include <string>
#include <netinet/in.h>

#include "xnet/base/macros.h"

namespace xnet {
namespace net {

class InetAddress {
 public:
  explicit InetAddress(uint16_t port);
  InetAddress(const std::string& ip, uint16_t port);
  InetAddress(const struct sockaddr_in& addr) : 
      addr_(addr) {}
  std::string ToHostPort() const;
  const struct sockaddr_in& GetSocketAddrInet() const { return addr_; }
  void SetSocketAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }
  
 private:
  static const in_addr_t kInAddrAny;
  struct sockaddr_in addr_;
};

}  // namespace net
}  // namespace xnet

#endif  // namespace  XNET_NET_INET_ADDRESS_H_