#include "xnet/net/timer.h"


namespace xnet {
namespace net {
void Timer::Restart(Timestamp now) {
  if (repeat_) {
    expiration_ = now + interval_;
  } else {
    expiration_ = Timestamp();
  }
}

}  // namespace net 
}  // namespace xnet
