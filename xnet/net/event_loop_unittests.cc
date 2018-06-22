#include <stdio.h>

#include "xnet/net/event_loop.h"
#include "xnet/net/channel.h"

#include "gtest/gtest.h"

namespace xnet {
namespace net {

EventLoop* g_loop;


void readcallback() {
  std::cout << "readcallback!\n";
  g_loop->Quit();
}

void ThreadFunc(EventLoop* loop) {
  loop->Loop();
}

/*
TEST(EventLoopTest, basic1) {
  EventLoop loop;
  std::thread thread(ThreadFunc, &loop);
  thread.join();
}
*/

TEST(EventLoopTest, basic2) {
  EventLoop loop;
  g_loop = &loop;
  Channel channel(&loop, STDIN_FILENO);
  channel.set_read_callback(readcallback);
  channel.EnableReading();
  loop.Loop();
}


}  // namespace net
}  // namespace xnet

