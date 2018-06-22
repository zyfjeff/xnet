#include <poll.h>
#include <assert.h>

#include "glog/logging.h"
#include "xnet/net/poller.h"
#include "xnet/net/channel.h"

namespace xnet {
namespace net {

Poller::Poller(EventLoop* loop) : own_loop_(loop) { }
Poller::~Poller() { }

std::chrono::milliseconds Poller::Poll(int timeout_ms,
                                      ChanneList* active_channels) {
  int num_events = ::poll(&*poll_fds_.begin(), poll_fds_.size(), timeout_ms);
  auto now = std::chrono::system_clock::now();
  if (num_events > 0) {
    LOG(INFO) << num_events << " events happended";
    FillActiveChannels(num_events, active_channels);
  } else if (num_events == 0) {
    LOG(INFO) << " nothing happended";
  } else {
    LOG(INFO) << "Poller::poll()";
  }

  return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
}

void Poller::FillActiveChannels(int num_events,
                                ChanneList* active_channels) const {
  for(PollFdList::const_iterator pfd = poll_fds_.begin();
      pfd != poll_fds_.end() && num_events > 0; ++pfd) {
    if (pfd->revents > 0) {
      --num_events;
      ChannelMap::const_iterator ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      active_channels->push_back(channel);

    }
  }
}

void Poller::UpdateChannel(Channel* channel) {
  AssertInLoopThread();
  LOG(INFO) << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) {
    // Add a new one，add to poll_fds_
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    poll_fds_.push_back(pfd);
    int idx = static_cast<int>(poll_fds_.size()) - 1;
    channel->set_index(idx);
    channels_[pfd.fd] = channel;
  } else {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(poll_fds_.size()));
    struct pollfd& pfd = poll_fds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -1);
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->IsNoneEvent()) {
      // ignore this pollfd
      pfd.fd = -1;
    }
  }
}



}  // namespace net
}  // namespace xnet