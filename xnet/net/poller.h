#ifndef XNET_SRC_BASE_POLLER_H_
#define XNET_SRC_BASE_POLLER_H_

#include <map>
#include <vector>
#include <chrono>

#include "xnet/net/event_loop.h"
#include "xnet/base/macros.h"

struct pollfd;

namespace xnet {
namespace net {

class Channel;

// Base class for IO Multiplexing
// This class doesn't own the Channel objects
class Poller {
 public:
  using ChanneList = std::vector<Channel*>;
  Poller(EventLoop* loop);
  ~Poller();
  std::chrono::milliseconds Poll(int timeout_ms, ChanneList* active_channels);
  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);
  bool HasChannel(Channel* channel) const;
  // static Poller* NewDefaultPoller(EventLoop* loop);
  void AssertInLoopThread() const {
    own_loop_->AssertInLoopThread();
  }

 private:
  void FillActiveChannels(int num_events, ChanneList* active_channels) const;
  using ChannelMap = std::map<int, Channel*>;
  using PollFdList = std::vector<struct pollfd>;

  PollFdList poll_fds_;
  ChannelMap channels_;
  EventLoop* own_loop_;

  DISALLOW_COPY_AND_ASSIGN(Poller);
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_SRC_BASE_POLLER_H_
