#ifndef MUDUO_NET_EVENT_LOOP_H
#define MUDUO_NET_EVENT_LOOP_H
#include <muduo/base/Thread.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/noncopyable.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TimerId.h>
#include <memory>

namespace muduo {
namespace net {

class Channel;
class Poller;
class TimerQueue;


class EventLoop :noncopyable{
public:
  
  EventLoop();
  ~EventLoop();

  void loop();

  void quit();

  ///
  /// Runs callback at 'time'.
  ///
  TimerId runAt(const Timestamp& time, const TimerCallback&& cb);
  ///
  /// Runs callback after @c delay seconds.
  ///
  TimerId runAfter(double delay, const TimerCallback&& cb);
  ///
  /// Runs callback every @c interval seconds.
  ///
  TimerId runEvery(double interval, const TimerCallback&& cb);

  // void cancel(TimerId timerId);



  // internal use only
  void updateChannel(Channel* channel);
  // void removeChannel(Channel* channel);

  void assertInLoopThread() {
    if (!isInloopThread()) {
      abortNotInLoopThread();
    }
  }

  bool isInloopThread() const 
  { return threadId_ == CurrentThread::tid(); }

  static EventLoop* getEventLoopOfCurrentThread();

 private:
  // EventLoop(const EventLoop&) = delete;
  // EventLoop& operator=(const EventLoop&) = delete;

  void abortNotInLoopThread();
  typedef std::vector<Channel*> ChannelList;
  bool looping_;/* atomic */
  bool quit_;/* atomic */
  const pid_t threadId_;
  
  //use const to avoid swap
  //http://stackoverflow.com/questions/30143413/changing-boostscoped-ptr-to-stdunique-ptr
  const std::unique_ptr<Poller> poller_;
  const std::unique_ptr<TimerQueue> timerQueue_;
  ChannelList activeChannels_;
};


}
}

#endif  // MUDUO_NET_EVENT_LOOP_H
