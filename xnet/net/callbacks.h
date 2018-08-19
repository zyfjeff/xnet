#ifndef XNET_NET_CALLBACKS_H_
#define XNET_NET_CALLBACKS_H_

#include <functional>
#include <memory>

#include "xnet/net/timestamp.h"
#include "xnet/net/buffer.h"

namespace xnet {
namespace net {

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using CloseCallback =  std::function<void (const TcpConnectionPtr&)>;
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&,
                                            Buffer* buf,
                                            Timestamp)>;
using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
}  // namespace net
}  // namespace xnet
#endif  // XNET_NET_CALLBACKS_H_