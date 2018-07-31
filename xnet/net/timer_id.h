#ifndef XNET_NET_TIMERID_H
#define XNET_NET_TIMERID_H


namespace xnet {
namespace net {
class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId
{
 public:
  explicit TimerId(Timer* timer)
    : value_(timer)
  {
  }

 private:
  Timer* value_;
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_TIMERID_H
