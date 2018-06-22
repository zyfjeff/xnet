#include "xnet/net/event_loop.h"

#include <poll.h>
#include "glog/logging.h"
#include "absl/base/macros.h"
#include "xnet/net/poller.h"
#include "xnet/net/channel.h"

namespace details {
thread_local xnet::net::EventLoop* gt_loop_in_this_thread = nullptr;
}

namespace xnet {
namespace net {

const int EventLoop::kPollTimeMs = 50000;

EventLoop::EventLoop() : looping_(false), 
    thread_id_(std::this_thread::get_id()), poller_(std::make_unique<Poller>(this)) {
  LOG(INFO) << "EventLoop created " << this << " in thread " << thread_id_;
  if (details::gt_loop_in_this_thread) {
    LOG(FATAL) << "Another EventLoop " << details::gt_loop_in_this_thread
               << " exists in this thread";
  } else {
    details::gt_loop_in_this_thread = this;
  }
}

EventLoop::~EventLoop() {
  ABSL_ASSERT(!looping_);
  details::gt_loop_in_this_thread = nullptr;
}

void EventLoop::Quit() {
  quit_ = true;
  // Wakeup
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
  }

  LOG(INFO) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

// static
EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
  return details::gt_loop_in_this_thread;
}

}  // namespace net
}  // namespace xnet