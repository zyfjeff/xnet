#include "xnet/net/tcp_server.h"

#include "xnet/net/inet_address.h"
#include "xnet/net/socket_utils.h"
#include "xnet/net/acceptor.h"
#include "xnet/net/event_loop.h"
#include "xnet/net/tcp_connection.h"
#include "xnet/net/event_loop_thread_pool.h"

#include <glog/logging.h>

namespace xnet {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(listen_addr.ToHostPort()),
      thread_pool_(std::make_unique<EventLoopThreadPool>(loop_)),
      acceptor_(std::make_unique<Acceptor>(loop_, listen_addr)),
      started_(false),
      next_connid_(1) {
  acceptor_->set_new_connection_callback(std::bind(&TcpServer::NewConnection, this, 
                                         std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::set_thread_num(int num) {
  assert(0 <= num);
  thread_pool_->set_thread_num(num);
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG(INFO) << "TcpServer::RemoveConnectionInLoop [" << name_
            << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  assert(n == 1);
  ignore_unused(n);

  EventLoop* loop = thread_pool_->GetNextLoop();
  loop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}

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
  EventLoop* loop = thread_pool_->GetNextLoop();
  TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop, conn_name, 
                                                        sockfd, local_addr, peer_addr));
  connections_[conn_name] = conn;
  conn->set_connection_callback(conn_callback_);
  conn->set_message_callback(msg_callback_);
  conn->set_write_complete_callback(write_complete_callback_);
  conn->set_close_callback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
  loop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
  conn->ConnectEstablished();
}

}  // namespace net
}  // namespace xnet