#include "xnet/net/tcp_server.h"

#include <glog/logging.h>
#include "xnet/net/inet_address.h"
#include "xnet/net/socket_utils.h"
#include "xnet/net/acceptor.h"
#include "xnet/net/event_loop.h"
#include "xnet/net/tcp_connection.h"

namespace xnet {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(listen_addr.ToHostPort()),
      acceptor_(std::make_unique<Acceptor>(loop_, listen_addr)),
      started_(false),
      next_connid_(1) {
  acceptor_->set_new_connection_callback(std::bind(&TcpServer::NewConnection, this, 
                                         std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::Start() {
  if (!started_) {
    started_ = true;
  }

  if (!acceptor_->Listenning()) {
    loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
}

void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
  loop_->AssertInLoopThread();
  char buf[32] = {0};
  snprintf(buf, sizeof buf, "#%d", next_connid_);
  ++next_connid_;
  std::string conn_name = name_ + buf;
  LOG(INFO) << "TcpServer::NewConnection [" << name_
            << "] - new connection [" << conn_name
            << "] from " << peer_addr.ToHostPort();
  InetAddress local_addr(SocketUtils::GetLocalAddr(sockfd));
  TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_, conn_name, 
                                                        sockfd, local_addr, peer_addr));
  connections_[conn_name] = conn;
  conn->set_connection_callback(conn_callback_);
  conn->set_message_callback(msg_callback_);
  conn->ConnectEstablished();
}

}  // namespace net
}  // namespace xnet