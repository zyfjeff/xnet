#include "xnet/net/channel.h"

#include <poll.h>

#include "glog/logging.h"
#include "xnet/net/event_loop.h"

namespace xnet {
namespace net {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::~Channel() {
  assert(!event_handling_);
}

Channel::Channel(EventLoop* loop, int fd) :
  loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1) { }

void Channel::Update() {
  loop_->UpdateChannel(this);
}

void Channel::HandleEvent() {
  event_handling_ = true;
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG(WARNING) << "Channel::handle_event() POLLHUP";
    if (close_callback_) {
      close_callback_();
    }
  }

  if (revents_ & POLLNVAL) {
    LOG(WARNING) << "Channel::HandleEvent() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_callback_) error_callback_();
  }

  if (revents_ & (POLLIN | POLLPRI | POLLHUP)) {
    if (read_callback_) read_callback_();
  }

  if (revents_ & POLLOUT) {
    if (write_callback_) write_callback_();
  }

  event_handling_ = false;
}

}  // namespace net
}  // namespace xnet