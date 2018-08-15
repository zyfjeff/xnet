#include "xnet/net/tcp_connection.h"

#include "xnet/net/socket_utils.h"
#include "xnet/net/event_loop.h"
#include "xnet/net/channel.h"
#include "glog/logging.h"

#include <error.h>

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

void TcpConnection::SetCloseConnection(const CloseCallback& cb) {
  close_callback_ = cb;
}

TcpConnection::~TcpConnection() {
  LOG(INFO) << "TcpConnection:dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd();
}

void TcpConnection::HandleRead() {
  char buf[65535] = {0};
  ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
  if (n > 0) {
    msg_callback_(shared_from_this(), buf, n);
  } else if (n == 0) {
    HandleClose();
  } else {
    HandleError();
  }
}

void TcpConnection::HandleWrite() {

}

void TcpConnection::HandleClose() {
  loop_->AssertInLoopThread();
  LOG(INFO) << "TcpConnection::HandleClose state = " << static_cast<int>(state_);
  assert(state_ == StateE::kConnected);
  channel_->DisableAll();
  close_callback_(shared_from_this());
}

void TcpConnection::HandleError() {
  int err = SocketUtils::GetSocketError(channel_->fd());
  LOG(ERROR) << "TcpConnection::HandleError [" << name_
             << "] - SO_ERROR = " << err << " " << strerror(err);
}

void TcpConnection::ConnectEstablished() {
  loop_->AssertInLoopThread();
  assert(state_ == StateE::kConnecting);
  SetState(StateE::kConnected);
  channel_->EnableReading();

  connection_callback_(shared_from_this());
}

void TcpConnection::ConnectDestroyed() {
  loop_->AssertInLoopThread();
  assert(state_ == StateE::kConnected);
  channel_->DisableAll();
  connection_callback_(shared_from_this());
  loop_->RemoveChannel(channel_.get());
}

}  // namespace net
}  // namespace xnet