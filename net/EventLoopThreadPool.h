

#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>


#include <vector>
#include <functional>
#include <muduo/base/noncopyable.h>
#include <memory>

namespace muduo
{
namespace net 
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
 public:
  EventLoopThreadPool(EventLoop* baseLoop);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start();
  EventLoop* getNextLoop();

 private:
  EventLoop* baseLoop_;
  bool started_;
  int numThreads_;
  int next_;  // always in loop thread
  std::vector<std::unique_ptr<EventLoopThread> > threads_;
  std::vector<EventLoop*> loops_;
};

} // net
}

#endif  // MUDUO_NET_EVENTLOOPTHREADPOOL_H
