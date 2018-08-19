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
  DLOG(INFO) << "TcpConnection::ctor[" << name_ << "] at" 
             << this << " fd=" << sockfd;
  channel_->set_read_callback(std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
  channel_->set_write_callback(std::bind(&TcpConnection::HandleWrite, this));
  channel_->set_close_callback(std::bind(&TcpConnection::HandleClose, this));
  channel_->set_error_callback(std::bind(&TcpConnection::HandleError, this));
}

TcpConnection::~TcpConnection() {
  LOG(INFO) << "TcpConnection:dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd();
}

void TcpConnection::HandleRead(Timestamp receive_time) {
  int err = 0;
  ssize_t n = input_buffer_.ReadFd(channel_->fd(), &err);
  if (n > 0) {
    msg_callback_(shared_from_this(), &input_buffer_, receive_time);
  } else if (n == 0) {
    HandleClose();
  } else {
    errno = err;
    LOG(FATAL) << "TcpConnectin::HandleRead";
    HandleError();
  }
}

void TcpConnection::HandleWrite() {
  loop_->AssertInLoopThread();
  if (channel_->IsWriting()) {
    LOG(INFO) << "Connection is down, no more writing";
    return;
  }

  ssize_t n = ::write(channel_->fd(),
                      output_buffer_.Peek(),
                      output_buffer_.ReadableBytes());
  if (n > 0) {
    output_buffer_.Retrieve(n);
    if (output_buffer_.ReadableBytes() == 0) {
      channel_->DisableWriting();
      if (write_complte_callback_) {
        loop_->QueueInLoop(
            std::bind(std::bind(write_complte_callback_, shared_from_this())));
      }

      if (state_ == StateE::kDisconnecting) {
        ShutdownInLoop();
      }
    } else {
      LOG(INFO) << "I am going to write more data";
    }
  } else {
    LOG(INFO) << "TcpConnection::HandleWrite";
  }
}

void TcpConnection::HandleClose() {
  loop_->AssertInLoopThread();
  LOG(INFO) << "TcpConnection::HandleClose state = " 
            << static_cast<int>(state_);
  assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
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
  assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
  channel_->DisableAll();
  connection_callback_(shared_from_this());
  loop_->RemoveChannel(channel_.get());
}

void TcpConnection::Send(const absl::string_view& str) {
  if (state_ == StateE::kConnected) {
    if (loop_->IsInLoopThread()) {
      SendInLoop(str);
    } else {
      loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, str));
    }
  }
}

void TcpConnection::Shutdown() {
  if (state_ == StateE::kConnected) {
    SetState(StateE::kDisconnecting);
    loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
  }
}

void TcpConnection::SendInLoop(const absl::string_view& str) {
  loop_->AssertInLoopThread();
  ssize_t nwrote = 0;
  if (!channel_->IsWriting() && output_buffer_.ReadableBytes() == 0) {
    nwrote = ::write(channel_->fd(), str.data(), str.size());
    if (nwrote >= 0) {
      if (static_cast<size_t>(nwrote) < str.size()) {
        LOG(INFO) << "I am going to write more data";
      } else if (write_complte_callback_) {
        loop_->QueueInLoop(std::bind(write_complte_callback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG(FATAL) << "TcpConnection::SendInLoop" << strerror(errno);
      }
    }
  }

  assert(nwrote >= 0);
  if (static_cast<size_t>(nwrote) < str.size()) {
    output_buffer_.Append(absl::string_view(str.data() + nwrote, str.size() - nwrote));
    if (!channel_->IsWriting()) {
      channel_->EnableWriteing();
    }
  }
}

void TcpConnection::ShutdownInLoop() {
  loop_->AssertInLoopThread();
  if (!channel_->IsWriting()) {
    socket_->ShutdownWrite();
  }
}

void TcpConnection::SetTcpNodelay(bool on) {
  socket_->SetTcpNoDelay(on);
}

}  // namespace net
}  // namespace xnet