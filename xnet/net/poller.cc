#include <poll.h>
#include <assert.h>

#include <chrono>

#include "glog/logging.h"
#include "xnet/net/poller.h"
#include "xnet/net/channel.h"

namespace xnet {
namespace net {

Poller::Poller(EventLoop* loop) : own_loop_(loop) { }
Poller::~Poller() { }

Timestamp Poller::Poll(int timeout_ms,
                                      ChanneList* active_channels) {
  LOG(INFO) << "Starting poll" << poll_fds_.size();
  int num_events = ::poll(&*poll_fds_.begin(), poll_fds_.size(), timeout_ms);
  auto now = Timestamp::clock::now();
  if (num_events > 0) {
    LOG(INFO) << num_events << " events happended";
    FillActiveChannels(num_events, active_channels);
  } else if (num_events == 0) {
    LOG(INFO) << " nothing happended";
  } else {
    LOG(INFO) << "Poller::poll()";
  }

  return now;
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

void Poller::RemoveChannel(Channel* channel) {
  AssertInLoopThread();
  LOG(INFO) << "delete: fd = " << channel->fd() << " events = " << channel->events();
  assert(channel->index() >= 0);
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->IsNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(poll_fds_.size()));
  const struct pollfd& pfd = poll_fds_[idx]; (void)pfd;
  assert(pfd.fd == channel->fd() - 1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1); (void)n;
  if (idx == poll_fds_.size() - 1) {
    poll_fds_.pop_back();
  } else {
    int channel_at_end = poll_fds_.back().fd;
    std::iter_swap(poll_fds_.begin() + idx, poll_fds_.end() - 1);
    if (channel_at_end < 0) {
      channel_at_end = channel_at_end - 1;
    }

    channels_[channel_at_end]->set_index(idx);
    poll_fds_.pop_back();
  }
}

void Poller::UpdateChannel(Channel* channel) {
  AssertInLoopThread();
  LOG(INFO) << "update: fd = " << channel->fd() << " events = " << channel->events();
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