#ifndef MUDUO_NET_TIMER_ID_H
#define MUDUO_NET_TIMER_ID_H

#include <muduo/base/copyable.h>

namespace muduo {
namespace net {
class Timer;
//where to put the TimerPtr????
typedef std::shared_ptr<Timer> TimerPtr;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public copyable {
 public:
  explicit TimerId(TimerPtr timer) : value_(timer) {}

  // default copy-ctor, dtor and assignment are okay

 private:
  TimerPtr value_;
};

}  // net
}  // muduo

#endif  // MUDUO_NET_TIMER_ID_H
