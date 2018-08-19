#ifndef XNET_NET_EVENT_LOOP_THREAD_POOL_H_
#define XNET_NET_EVENT_LOOP_THREAD_POOL_H_

#include <vector>

namespace xnet {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  EventLoopThreadPool(EventLoop* loop);
  ~EventLoopThreadPool();
  void set_thread_num(int num_thread) { thread_num_ = num_thread; }
  void Start();
  EventLoop* GetNextLoop();

 private:
  EventLoop* loop_;
  int thread_num_;
  bool started_;
  int next_;
  std::vector<EventLoopThread*> threads_;
  std::vector<EventLoop*> loops_;
};

}  // namespace net
}  // namespace xnet


#endif  // XNET_NET_EVENT_LOOP_THREAD_POOL_H_