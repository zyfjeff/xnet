#ifndef XNET_NET_TCP_CONNECTION_H_
#define XNET_NET_TCP_CONNECTION_H_

#include <memory>

#include "xnet/net/socket.h"
#include "xnet/net/callbacks.h"
#include "xnet/net/inet_address.h"

namespace xnet {
namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name,
                int sockfd, const InetAddress& local_addr,
                const InetAddress& peer_addr);
  ~TcpConnection();
  EventLoop* loop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& LocalAddress() { return local_addr_; }
  bool Connected() const { return state_ == StateE::kConnected; }

  void set_connection_callback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
  }

  void set_message_callback(const MessageCallback& cb) {
    msg_callback_ = cb;
  }

  void ConnectEstablished();
  void ConnectDestroyed();

 private:
  enum class StateE { kConnecting, kConnected, };
  void SetState(StateE s) { state_ = s; }
  void HandleRead();
  void HandleWrite();
  void HandleClose();
  void HandleError();
  void SetCloseConnection(const CloseCallback& cb);

  EventLoop* loop_;
  std::string name_;
  StateE state_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress local_addr_;
  InetAddress peer_addr_;

  ConnectionCallback connection_callback_;
  MessageCallback msg_callback_;
  CloseCallback close_callback_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_TCP_CONNECTION_H_