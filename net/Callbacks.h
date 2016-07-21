#ifndef MUDUO_NETD_CALLBACKS_H
#define MUDUO_NETD_CALLBACKS_H

#include <muduo/base/Timestamp.h>
#include <functional>
#include <memory>

namespace muduo {
namespace net {

class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;

typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, const char* data,
                           ssize_t len)> MessageCallback;


typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
}
}

#endif  // MUDUO_NETD_CALLBACKS_H
