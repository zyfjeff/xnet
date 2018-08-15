#ifndef XNET_NET_CHANNEL_H_
#define XNET_NET_CHANNEL_H_

#include <functional>

#include "xnet/base/macros.h"

namespace xnet {
namespace net {

class EventLoop;

/// A selectable I/O channel
/// This class doesn't own the file descriptor
class Channel {
 public:
  using EventCallback = std::function<void()>;
  Channel(EventLoop* loop, int fd);
  ~Channel();
  void HandleEvent();
  void set_read_callback(const EventCallback& cb) {
    read_callback_ = cb;
  }
  void set_write_callback(const EventCallback& cb) {
    write_callback_ = cb;
  }
  void set_error_callback(const EventCallback& cb) {
    error_callback_ = cb;
  }
  void set_close_callback(const EventCallback& cb) {
    close_callback_ = cb;
  }

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }
  bool IsNoneEvent() const { return events_ == kNoneEvent; }

  void Update();
  void EnableReading() { events_ |= kReadEvent; Update(); }
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }
  EventLoop* loop() { return loop_; }
  void DisableAll() { return ; }

 private:
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_;
  bool event_handling_{false};

  EventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback error_callback_;
  EventCallback close_callback_;

  DISALLOW_COPY_AND_ASSIGN(Channel);
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_CHANNEL_H_
