#include <stdio.h>
#include <chrono>
#include <sys/timerfd.h>

#include "xnet/net/event_loop.h"
#include "xnet/net/timer_queue.h"
#include "xnet/net/timer_id.h"
#include "xnet/net/channel.h"

#include "gtest/gtest.h"

namespace xnet {
namespace net {

EventLoop* g_loop;


void timeout() {
  std::cout << "timeout!\n";
  g_loop->Quit();
}

void readcallback() {
  std::cout << "readcallback!\n";
  g_loop->Quit();
}

void ThreadFunc(EventLoop* loop) {
  loop->Loop();
}

TEST(EventLoopTest, basic1) {
  EventLoop loop;
  g_loop = &loop;
  Channel channel(&loop, STDIN_FILENO);
  channel.set_read_callback(readcallback);
  channel.EnableReading();
  loop.Loop();
}

TEST(EventLoopTest, basic2) {
  EventLoop loop;
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  Channel channel(&loop, timerfd);
  channel.set_read_callback(timeout);
  channel.EnableReading();

  struct itimerspec howlong;
  bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 1;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);
  loop.Loop();
}

TEST(EventLoopTest, DISABLED_basic3) {
  EventLoop loop;
  std::thread thread(ThreadFunc, &loop);
  thread.join();
}

TEST(EventLoopTest, basic4) {
  EventLoop loop;
  loop.RunAfter(std::chrono::milliseconds(1000), [](){
    std::cout << "once1" << std::endl;
  });

  loop.RunEvery(std::chrono::milliseconds(3000), [](){
    std::cout << "once3" << std::endl;
  });

  loop.Loop();
}


}  // namespace net
}  // namespace xnet

