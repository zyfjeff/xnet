#ifndef XNET_NET_TCP_CONNECTION_H_
#define XNET_NET_TCP_CONNECTION_H_

#include <memory>

#include "xnet/net/socket.h"
#include "xnet/net/callbacks.h"
#include "xnet/net/inet_address.h"
#include "xnet/net/buffer.h"

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

  void set_write_complete_callback(const WriteCompleteCallback& cb) {
    write_complte_callback_ = cb;
  }

  void set_close_callback(const CloseCallback& cb) {
    close_callback_ = cb;
  }

  void ConnectEstablished();
  void ConnectDestroyed();

  void Send(const absl::string_view& str);
  void Shutdown();

  void SetTcpNodelay(bool on);

 private:
  enum class StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };
  void SetState(StateE s) { state_ = s; }
  void HandleRead(Timestamp receive_time);
  void HandleWrite();
  void HandleClose();
  void HandleError();
  void SendInLoop(const absl::string_view& str);
  void ShutdownInLoop();

  EventLoop* loop_;
  std::string name_;
  StateE state_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress local_addr_;
  InetAddress peer_addr_;

  ConnectionCallback connection_callback_{nullptr};
  MessageCallback msg_callback_{nullptr};
  CloseCallback close_callback_{nullptr};
  WriteCompleteCallback write_complte_callback_{nullptr};

  Buffer input_buffer_;
  Buffer output_buffer_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_TCP_CONNECTION_H_