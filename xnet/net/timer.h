#ifndef XNET_NET_TIMER_H
#define XNET_NET_TIMER_H

#include <functional>

#include "xnet/net/timestamp.h"
#include "xnet/net/callbacks.h"

namespace xnet {
namespace net {

class Timer {
 public:
  Timer(const TimerCallback& cb, Timestamp when, std::chrono::milliseconds interval)
    : callback_(cb),
      expiration_(when),
      interval_(interval),
      repeat_(interval > std::chrono::milliseconds(0))
  { }

  ~Timer() {}
  
  void Run() const {
    callback_();
  }

  Timestamp Expiration() const  { return expiration_; }
  bool Repeat() const { return repeat_; }

  void Restart(Timestamp now);

 private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const std::chrono::milliseconds interval_;
  const bool repeat_;
};

}  // namespace net
}  // namespace xnet
#endif  // XNET_NET_TIMER_H
