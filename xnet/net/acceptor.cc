#include "xnet/net/acceptor.h"

#include "xnet/net/socket_utils.h"
#include "xnet/net/event_loop.h"

namespace xnet {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr) :
    loop_(loop),
    accept_socket_(SocketUtils::CreateNonblockingOrDie()),
    accept_channel_(loop_, accept_socket_.sockfd()),
    listenning_(false) {
  accept_socket_.SetReuseAddr(true);
  accept_socket_.BindAddress(listen_addr);
  accept_channel_.set_read_callback(std::bind(&Acceptor::HandleRead, this));
}

void Acceptor::Listen() {
  loop_->AssertInLoopThread();
  listenning_ = true;
  accept_socket_.Listen();
  accept_channel_.EnableReading();
}

void Acceptor::HandleRead() {
  loop_->AssertInLoopThread();
  InetAddress peer_addr(0);
  // TODO(tianqian.zyf): loop until no more
  int connfd = accept_socket_.Accept(&peer_addr);
  if (connfd >= 0) {
    if (callback_) {
      // TODO(tianqian.zyf): Use Socket object replace connfd
      callback_(connfd, peer_addr);
    }
  } else {
    SocketUtils::Close(connfd);
  }
}

Acceptor::~Acceptor() {}


}  // namespace net
}  // namespace xnet