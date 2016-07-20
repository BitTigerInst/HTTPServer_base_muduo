#ifndef MUDUO_NET_EVENT_LOOP_THREAD_H
#define MUDUO_NET_EVENT_LOOP_THREAD_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/noncopyable.h>

namespace muduo {
namespace net {

class EventLoop;

class EventLoopThread:noncopyable
{
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop* startLoop();

private:
	void threadFunc();

	EventLoop* loop_;
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
};



} // net
} // muduo








#endif // MUDUO_NET_EVENT_LOOP_THREAD_H
