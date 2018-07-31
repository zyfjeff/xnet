#include "xnet/net/event_loop.h"

#include <sys/eventfd.h>
#include <poll.h>
#include "glog/logging.h"
#include "absl/base/macros.h"
#include "xnet/net/poller.h"
#include "xnet/net/channel.h"
#include "xnet/net/timer_queue.h"
#include "xnet/net/timer_id.h"

namespace details {
thread_local xnet::net::EventLoop* gt_loop_in_this_thread = nullptr;
}

namespace xnet {
namespace net {

const int EventLoop::kPollTimeMs = 50000;

EventLoop::EventLoop() : looping_(false), 
    thread_id_(std::this_thread::get_id()), 
    poller_(std::make_unique<Poller>(this)),
    timer_queue_(std::make_unique<TimerQueue>(this)),
    wakeup_fd_(eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK)), 
    wakeup_channel_(std::make_unique<Channel>(this, wakeup_fd_)) {
  LOG(INFO) << "EventLoop created " << this << " in thread " << thread_id_;
  if (details::gt_loop_in_this_thread) {
    LOG(FATAL) << "Another EventLoop " << details::gt_loop_in_this_thread
               << " exists in this thread";
  } else {
    details::gt_loop_in_this_thread = this;
  }

  wakeup_channel_->set_read_callback(std::bind(&EventLoop::HandleRead, this));
  wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
  ABSL_ASSERT(!looping_);
  details::gt_loop_in_this_thread = nullptr;
}


void EventLoop::DoPendingFunctors() {
  std::vector<Functor> functors;
  // TODO(tianqian.zyf): calling_pending_functors_ not thread safely
  calling_pending_functors_ = true;

  {
    std::unique_lock<std::mutex> lk(mtx_);
    functors.swap(pending_functors_);
  }

  for(size_t i = 0; i < functors.size(); ++i) {
    functors[i]();
  }

  calling_pending_functors_ = false;
}


void EventLoop::QueueInLoop(const Functor& cb) {
  {
    std::unique_lock<std::mutex> lk(mtx_);
    pending_functors_.push_back(cb);
  }

  if (!IsInLoopThread() || calling_pending_functors_) {
    Wakeup();
  }
}

void EventLoop::RunInLoop(const Functor& cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueueInLoop(cb);
  }
}

void EventLoop::Wakeup() {
  uint64_t u = 1;
  write(wakeup_fd_, &u, sizeof(uint64_t));
}

void EventLoop::Quit() {
  quit_ = true;
  if (!IsInLoopThread()) {
    Wakeup();
  }
}

void EventLoop::UpdateChannel(Channel* channel) {
  assert(channel->loop() == this);
  AssertInLoopThread();
  poller_->UpdateChannel(channel);
}

void EventLoop::Loop() {
  ABSL_ASSERT(!looping_);
  AssertInLoopThread();
  looping_ = true;
  quit_ = false;

  while(!quit_) {
    auto now = poller_->Poll(kPollTimeMs, &active_channels_);
    for(ChannelList::iterator it = active_channels_.begin();
        it != active_channels_.end(); ++it) {
      (*it)->HandleEvent();
    }
    DoPendingFunctors();
  }

  LOG(INFO) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

// static
EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
  return details::gt_loop_in_this_thread;
}

TimerId EventLoop::RunAt(const Timestamp& time, 
                         const TimerCallback& cb) {
  return timer_queue_->AddTimer(cb, time, std::chrono::milliseconds(0));
}

TimerId EventLoop::RunAfter(std::chrono::milliseconds delay, 
                            const TimerCallback& cb) {
  return RunAt(std::chrono::system_clock::now() + delay, cb);
}


TimerId EventLoop::RunEvery(std::chrono::milliseconds interval, 
                            const TimerCallback& cb) {
  return timer_queue_->AddTimer(cb, 
      std::chrono::system_clock::now() + interval, interval);
}

}  // namespace net
}  // namespace xnet