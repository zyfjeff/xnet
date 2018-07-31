#ifndef XNET_NET_CALLBACKS_H_
#define XNET_NET_CALLBACKS_H_

#include <functional>
#include <memory>

namespace xnet {
namespace net {

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&,
                                            const char* data, ssize_t len)>;
}  // namespace net
}  // namespace xnet
#endif  // XNET_NET_CALLBACKS_H_