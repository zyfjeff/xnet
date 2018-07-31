#include "xnet/net/tcp_connection.h"

#include "xnet/net/event_loop.h"
#include "xnet/net/channel.h"
#include "glog/logging.h"

namespace xnet {
namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
                             int sockfd, const InetAddress& local_addr,
                             const InetAddress& peer_addr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(name),
      state_(StateE::kConnecting),
      socket_(std::make_unique<Socket>(sockfd)),
      channel_(std::make_unique<Channel>(loop, sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr) {

}

TcpConnection::~TcpConnection() {
  LOG(INFO) << "TcpConnection:dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd();
}

void TcpConnection::HandleRead() {
  char buf[65535] = {0};
  ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
  msg_callback_(shared_from_this(), buf, n);
  // TODO(tianqian.zyf): close connection if n == 0
}

void TcpConnection::ConnectEstablished() {
  loop_->AssertInLoopThread();
  assert(state_ == StateE::kConnecting);
  SetState(StateE::kConnected);
  channel_->EnableReading();

  connection_callback_(shared_from_this());
}

}  // namespace net
}  // namespace xnet