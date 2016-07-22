#ifndef MUDUO_NET_EVENT_LOOP_H
#define MUDUO_NET_EVENT_LOOP_H
#include <muduo/base/Thread.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/noncopyable.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TimerId.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Timestamp.h>
#include <memory>


namespace muduo {
namespace net {

class Channel;
class Poller;
class TimerQueue;


class EventLoop :noncopyable{
public:
  typedef std::function<void()> Functor;
  
  EventLoop();
  ~EventLoop();

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();

  void quit();

  ///
  /// Time when poll returns, usually means data arrivial.
  ///
  Timestamp pollReturnTime() const { return pollReturnTime_; }

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(Functor&& cb);
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(Functor&& cb);
  //^^ rvalue cannot use const

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
  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const 
  { return threadId_ == CurrentThread::tid(); }

  static EventLoop* getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  void handleRead();
  void doPendingFunctors();


  typedef std::vector<Channel*> ChannelList;
  bool looping_;/* atomic */
  bool quit_;/* atomic */
  bool callingPendingFunctors_;  /*atomic*/
  const pid_t threadId_;
  Timestamp pollReturnTime_;
  int wakeupFd_;
  
  //use const to avoid swap
  //http://stackoverflow.com/questions/30143413/changing-boostscoped-ptr-to-stdunique-ptr
  const std::unique_ptr<Poller> poller_;
  const std::unique_ptr<TimerQueue> timerQueue_;
  ChannelList activeChannels_;

  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  const std::unique_ptr<Channel> wakeupChannel_;
  MutexLock mutex_;
  std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_
};


}
}

#endif  // MUDUO_NET_EVENT_LOOP_H
