#include "xnet/net/event_loop_thread_pool.h"

#include "xnet/net/event_loop.h"
#include "xnet/net/event_loop_thread.h"

namespace xnet {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop)
    : loop_(loop), started_(false), thread_num_(0), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
  // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::Start() {
  assert(!started_);
  loop_->AssertInLoopThread();
  started_ = true;

  for(int i = 0; i < thread_num_; ++i) {
    EventLoopThread* t = new EventLoopThread;
    threads_.push_back(t);
    loops_.push_back(t->StartLoop());
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  loop_->AssertInLoopThread();
  EventLoop* loop = loop_;
  if (!loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}



}  // namespace net
}  // namespace xnet