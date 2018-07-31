#ifndef XNET_NET_EVENT_LOOP_H_
#define XNET_NET_EVENT_LOOP_H_

#include <thread>
#include <vector>
#include <cassert>
#include <mutex>
#include <functional>

#include "xnet/net/timestamp.h"
#include "xnet/net/callbacks.h"
#include "xnet/base/macros.h"

namespace xnet {
namespace net {

class Channel;
class Poller;
class TimerId;
class TimerQueue;

class EventLoop {
 public:
  using Functor = std::function<void(void)>;

  EventLoop();
  ~EventLoop();

  void Loop();
  void Quit();

  void AssertInLoopThread() {
    if (!IsInLoopThread()) {
      AbortNotInLoopThread();
    }
  }

  TimerId RunAt(const Timestamp& time, const TimerCallback& cb);
  TimerId RunAfter(std::chrono::milliseconds delya, const TimerCallback& cb);
  TimerId RunEvery(std::chrono::milliseconds delay, const TimerCallback& cb);
  void HandleRead() {}  // wake up
  void Wakeup();
  void DoPendingFunctors();
  void QueueInLoop(const Functor& cb);
  void RunInLoop(const Functor& cb);
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
  bool calling_pending_functors_;
  const std::thread::id thread_id_;
  ChannelList active_channels_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  std::mutex mtx_;
  std::vector<Functor> pending_functors_;
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;

  DISALLOW_COPY_AND_ASSIGN(EventLoop);
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_EVENT_LOOP_H_