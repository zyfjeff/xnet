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

void ThreadFunc() {
  EventLoop loop;
  loop.Loop();
}

TEST(EventLoopTest, basic1) {
  EventLoop loop;
  std::thread thread(ThreadFunc);
  thread.join();
}

/*
TEST(EventLoopTest, basic2) {
  EventLoop loop;
  g_loop = &loop;
  int stdin_fd = 0;
  Channel channel(&loop, stdin_fd);
  channel.set_read_callback(readcallback);
  channel.EnableReading();
  loop.Loop();
}
*/

}  // namespace net
}  // namespace xnet

