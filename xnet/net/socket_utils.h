#ifndef XNET_NET_SOCKETUTILS_H
#define XNET_NET_SOCKETUTILS_H

#include <arpa/inet.h>
#include <endian.h>

namespace xnet {
namespace net {

class SocketUtils {
 public:
  static inline uint64_t HostToNetwork64(uint64_t host64) {
    return htobe64(host64);
  }

  static inline uint32_t HostToNetwork32(uint32_t host32) {
    return htonl(host32);
  }

  static inline uint16_t HostToNetwork16(uint16_t host16) {
    return htons(host16);
  }

  static inline uint64_t NetworkToHost64(uint64_t net64) {
    return be64toh(net64);
  }

  static inline uint32_t NetworkToHost32(uint32_t net32) {
    return ntohl(net32);
  }

  static inline uint16_t NetworkToHost16(uint16_t net16) {
    return ntohs(net16);
  }

  static const sockaddr* sockaddr_cast(const struct sockaddr_in* addr) {
    return reinterpret_cast<const sockaddr*>(addr);
  }

  static sockaddr* sockaddr_cast(struct sockaddr_in* addr) {
    return reinterpret_cast<sockaddr*>(addr);
  }

  static int CreateNonblockingOrDie();
  static struct sockaddr_in GetLocalAddr(int sockfd);
  static int GetSocketError(int sockfd);
  static void BindOrDie(int sockfd, const struct sockaddr_in* addr);
  static void ListenOrDie(int sockfd);
  static int  Accept(int sockfd, struct sockaddr_in* addr);
  static void Close(int sockfd);
  static void ToHostPort(char* buf, size_t size,
                         const struct sockaddr_in& addr);
  static void FromHostPort(const char* ip, uint16_t port,
                           struct sockaddr_in* addr);
 private:
};


}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_SOCKETUTILS_H
