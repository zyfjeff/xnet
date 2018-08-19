#ifndef XNET_NET_TCP_SERVER_H_
#define XNET_NET_TCP_SERVER_H_

#include <map>

#include "xnet/net/callbacks.h"
#include "xnet/base/macros.h"

namespace xnet {
namespace net {

class Acceptor;
class EventLoop;
class InetAddress;
class EventLoopThreadPool;

class TcpServer {
 public:
  TcpServer(EventLoop* loop, const InetAddress& listen_addr);
  ~TcpServer();

  void Start();
  void set_connection_callback(const ConnectionCallback& cb) { conn_callback_ = cb; }
  void set_message_callback(const MessageCallback& cb) { msg_callback_ = cb; }
  void set_write_complete_callback(const WriteCompleteCallback& cb) { write_complete_callback_ = cb; }
  void set_thread_num(int num);

 private:
  void NewConnection(int sockfd, const InetAddress& peer_addr);
  void RemoveConnection(const TcpConnectionPtr& conn);
  void RemoveConnectionInLoop(const TcpConnectionPtr& conn);

  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

  EventLoop* loop_;
  const std::string name_;
  std::unique_ptr<EventLoopThreadPool> thread_pool_;
  std::unique_ptr<Acceptor> acceptor_;
  bool started_;
  int next_connid_;
  
  ConnectionMap connections_;
  ConnectionCallback conn_callback_{nullptr};
  MessageCallback msg_callback_{nullptr};
  WriteCompleteCallback write_complete_callback_{nullptr};
};


}  // namespace net
}  // namespace xnet
#endif  // XNET_NET_TCP_SERVER_H_