#ifndef MUDUO_NET_TIMER_QUEUE_H
#define MUDUO_NET_TIMER_QUEUE_H

#include <muduo/base/Mutex.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/noncopyable.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/Channel.h>
#include <memory>
#include <set>
#include <vector>

namespace muduo {
namespace net {
class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///

class TimerQueue : noncopyable {
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();
  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  TimerId addTimer(const TimerCallback&& cb, Timestamp when, double interval);

  void cancel(TimerId timerId);


 private:
  //shared with TimerID
  typedef std::shared_ptr<Timer> TimerPtr;

  typedef std::pair<Timestamp, TimerPtr> Entry;
  typedef std::set<Entry> TimerList;
  typedef std::set<Entry> CancelTimerList;//used for cancel self;

  void addTimerInLoop(TimerPtr timer);

  void cancelInLoop(TimerId timerId);

  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);

  bool insert(TimerPtr timer);

  EventLoop* loop_;
  const int timerfd_;
  
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;
  //to cancel;
  CancelTimerList cancelingTimers_;
  bool callingExpiredTimers_;

};


}
}

#endif  // MUDUO_NET_TIMER_QUEUE_H
