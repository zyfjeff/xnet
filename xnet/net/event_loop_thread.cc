#include "xnet/net/event_loop_thread.h"

#include <assert.h>

#include "xnet/net/event_loop.h"

namespace xnet {
namespace net {

EventLoopThread::EventLoopThread() {}
EventLoopThread::~EventLoopThread() {}

EventLoop* EventLoopThread::StartLoop() {
  assert(!th_.get());
  th_ = std::make_unique<std::thread>(
      std::bind(&EventLoopThread::ThreadFunc, this));
  { 
    std::unique_lock<std::mutex> lock(mutex_);
    while(loop_ == nullptr) {
      cv_.wait(lock);
    }
  }

  return loop_;
}

void EventLoopThread::ThreadFunc() {
  EventLoop loop;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = &loop;
    cv_.notify_one();
  }
  loop.Loop();
}

}  // namespace net
}  // namespace xnet