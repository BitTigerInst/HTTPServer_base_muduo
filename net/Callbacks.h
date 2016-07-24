#ifndef MUDUO_NETD_CALLBACKS_H
#define MUDUO_NETD_CALLBACKS_H

#include <muduo/base/Timestamp.h>
#include <functional>
#include <memory>

namespace muduo {
namespace net {
class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;

typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer* buf,
                           Timestamp)> MessageCallback;


typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
}
}

#endif  // MUDUO_NETD_CALLBACKS_H
