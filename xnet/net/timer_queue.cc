#include <cassert>
#include <algorithm>
#include <iostream>
#include <functional>

#include "glog/logging.h"
#include "xnet/net/event_loop.h"
#include "xnet/net/timer_queue.h"
#include "xnet/net/timer_id.h"
#include "xnet/net/timer.h"

namespace details {

int CreateTimerfd();
void ReadTimerfd(int timerfd, Timestamp now);
void ResetTimerfd(int timerfd, Timestamp expiration);

int CreateTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG(FATAL) << "Failed in timerfd_create";
  }
  return timerfd;
}

void ReadTimerfd(int timerfd, Timestamp now)
{
  uint64_t many;
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  size_t n = ::read(timerfd, &many, sizeof many);
  LOG(INFO) << "TimerQueue::HandleRead() " << many << " at " 
            << std::ctime(&time);
  if (n != sizeof many) {
    LOG(ERROR) << "TimerQueue::HandleRead() reads " << n << " bytes instead of 8";
  }
}


void ResetTimerfd(int timerfd, Timestamp expiration)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  struct timespec t;
  t.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(
      expiration - std::chrono::system_clock::now()).count();
  newValue.it_value = t;
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret) {
    LOG(ERROR) << "timerfd_settime()";
  }
}

}  // namespace details

namespace xnet {
namespace net {

TimerQueue::TimerQueue(EventLoop* loop) : loop_(loop),
    timer_fd_(details::CreateTimerfd()),
    timer_fd_channel_(loop, timer_fd_) {
  timer_fd_channel_.set_read_callback(std::bind(&TimerQueue::HandleRead, this));
  timer_fd_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now) {
  
  std::vector<Entry> expired;
  Entry sentry = std::make_pair(now, nullptr);
  TimerList::iterator it = timers_.lower_bound(sentry);
  assert(it == timers_.end() || now < it->first);
  
  for(auto i = timers_.begin(); i != it; ++i) {
    expired.push_back(std::move(const_cast<Entry &>(*i)));
  }
  
  timers_.erase(timers_.begin(), it);
  return expired;
}

void TimerQueue::HandleRead() {
  loop_->AssertInLoopThread();
  auto now = std::chrono::system_clock::now();
  details::ReadTimerfd(timer_fd_, now);
  std::vector<Entry> expired = GetExpired(now);

  for(auto& entry : expired) {
    entry.second->Run();
  }

  Reset(expired, now);
}

void TimerQueue::Reset(std::vector<Entry>& expired, Timestamp now) {
  Timestamp next_expire;
  for(auto& entry : expired) {
    if (entry.second->Repeat()) {
      entry.second->Restart(now);
      Insert(std::move(entry.second));
    } else {
      entry.second.release();
    }
  }

  if (!timers_.empty()) {
    next_expire = timers_.begin()->second->Expiration();
  }

  details::ResetTimerfd(timer_fd_, next_expire);
}

bool TimerQueue::Insert(std::unique_ptr<Timer> timer) {
  Timestamp when = timer->Expiration();
  auto result = timers_.emplace(std::make_pair(when, std::move(timer)));
  return result.second;
}

TimerId TimerQueue::AddTimer(const TimerCallback& cb, Timestamp when, 
                             std::chrono::milliseconds interval) {
  auto timer = std::make_unique<Timer>(cb, when, interval);
  Timer* ptimer = timer.get();

  /*
  auto f = std::bind([this](std::unique_ptr<Timer>& data) mutable {
    AddTimerInLoop(std::move(data));
  }, std::move(timer));
  */
  // Bug: 
  // loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, std::move(timer)));
  return TimerId(ptimer);
}

void TimerQueue::AddTimerInLoop(std::unique_ptr<Timer>&& timer)
{
  loop_->AssertInLoopThread();
  bool earliestChanged = Insert(std::move(timer));

  if (earliestChanged) {
    details::ResetTimerfd(timer_fd_, timer->Expiration());
  }
}



}  // namespace net
}  // namespace xnet