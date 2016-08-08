#ifndef MUDUO_NET_TIMER_ID_H
#define MUDUO_NET_TIMER_ID_H

#include <muduo/base/copyable.h>
#include <memory>

namespace muduo {
namespace net {
class Timer;
//where to put the TimerPtr????
typedef std::shared_ptr<Timer> TimerPtr;
typedef std::weak_ptr<Timer> TimerWeakPtr;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public copyable {
 public:
  TimerId(TimerPtr timer = NULL) : value_(timer) {}

  // default copy-ctor, dtor and assignment are okay

 friend class TimerQueue;
 private:
  TimerWeakPtr value_;
};

}  // net
}  // muduo

#endif  // MUDUO_NET_TIMER_ID_H
