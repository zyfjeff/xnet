#ifndef XNET_NET_EVENT_LOOP_THREAD_H_
#define XNET_NET_EVENT_LOOP_THREAD_H_

#include <thread>
#include <mutex>
#include <condition_variable>

#include "xnet/base/macros.h"

namespace xnet {
namespace net {

class EventLoop;

class EventLoopThread {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* StartLoop();

 private:

  void ThreadFunc();
  std::mutex mutex_;
  std::condition_variable cv_;
  std::unique_ptr<std::thread> th_;
  EventLoop* loop_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(EventLoopThread);
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_EVENT_LOOP_THREAD_H_