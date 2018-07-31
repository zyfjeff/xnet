#ifndef XNET_NET_ACCEPTOR_H_
#define XNET_NET_ACCEPTOR_H_

#include <functional>

#include "xnet/base/macros.h"
#include "xnet/net/socket.h"
#include "xnet/net/channel.h"
#include "xnet/net/inet_address.h"

namespace xnet {
namespace net {

class EventLoop;

class Acceptor {
 public:
  using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
  ~Acceptor();
  Acceptor(EventLoop* loop, const InetAddress& listen_addr);

  void set_new_connection_callback(const NewConnectionCallback& callback) {
    callback_ = callback;
  }

  bool Listenning() const { return listenning_; }
  void Listen();

 private:
  void HandleRead();

  EventLoop *loop_;
  Socket accept_socket_;
  Channel accept_channel_;
  bool listenning_;
  NewConnectionCallback callback_;
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_ACCEPTOR_H_