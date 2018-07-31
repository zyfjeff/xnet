#ifndef XNET_NET_TIMER_QUEUE_H_
#define XNET_NET_TIMER_QUEUE_H_

#include <memory>
#include <vector>
#include <chrono>
#include <set>
#include <sys/timerfd.h>

#include "xnet/net/channel.h"
#include "xnet/net/timestamp.h"
#include "xnet/net/callbacks.h"

namespace xnet {
namespace net {

class EventLoop;
class Timer;
class TimerId;
class Channel;

class TimerQueue {
 public:
  using Entry = std::pair<Timestamp, std::unique_ptr<Timer>>;
  using TimerList = std::set<Entry>;

  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  void AddTimerInLoop(std::unique_ptr<Timer>&& timer);
  TimerId AddTimer(const TimerCallback& cb, Timestamp when, 
                   std::chrono::milliseconds interval);

  std::vector<Entry> GetExpired(Timestamp now);
  void Reset(std::vector<Entry>& expired, Timestamp now);
  bool Insert(std::unique_ptr<Timer> timer);
 private:
  void HandleRead();
  
  EventLoop *loop_;
  int timer_fd_;
  Channel timer_fd_channel_;
  TimerList timers_;
};

}  // namespace net
}  // namespace xnet
#endif  // XNET_NET_TIMER_QUEUE_H_
