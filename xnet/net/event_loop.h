#ifndef XNET_NET_EVENT_LOOP_H_
#define XNET_NET_EVENT_LOOP_H_

#include <thread>
#include <vector>
#include <cassert>

#include "xnet/base/macros.h"

namespace xnet {
namespace net {

class Channel;
class Poller;

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  void Loop();
  void Quit();

  void AssertInLoopThread() {
    if (!IsInLoopThread()) {
      AbortNotInLoopThread();
    }
  }

  void UpdateChannel(Channel* channel);
  bool IsInLoopThread() const { return thread_id_ == std::this_thread::get_id(); }
  static EventLoop* GetEventLoopOfCurrentThread();
 private:
  static const int kPollTimeMs;
  using ChannelList = std::vector<Channel*>;

  void AbortNotInLoopThread() {
    assert(false);
  }
  bool looping_;
  bool quit_;
  const std::thread::id thread_id_;
  ChannelList active_channels_;
  std::unique_ptr<Poller> poller_;

  DISALLOW_COPY_AND_ASSIGN(EventLoop);
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_EVENT_LOOP_H_