#ifndef MUDUO_NETD_CALLBACKS_H
#define MUDUO_NETD_CALLBACKS_H

#include <functional>
#include <memory>
#include <muduo/base/Timestamp.h>

namespace muduo
{
namespace net
{
	typedef std::function<void()> TimerCallback;
}
}


#endif // MUDUO_NETD_CALLBACKS_H
