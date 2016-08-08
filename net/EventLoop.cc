
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Poller.h>
#include <muduo/net/TimerQueue.h>
#include <sys/eventfd.h>

#include <assert.h>
#include <poll.h>
#include <signal.h>

using namespace muduo;
using namespace muduo::net;

// gcc  local thread store
__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

static int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

IgnoreSigPipe initObj;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      wakeupFd_(createEventfd()),
      poller_(new Poller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupChannel_(new Channel(this, wakeupFd_)) {
  LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }

  // toregister wakeupChannel
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  assert(!looping_);
  t_loopInThisThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (ChannelList::iterator it = activeChannels_.begin();
         it != activeChannels_.end(); ++it) {
      (*it)->handleEvent(pollReturnTime_);
    }
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor&& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor&& cb) {
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback&& cb) {
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback&& cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, const TimerCallback&& cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
  return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    // reduce the lenth of the critical section
    // avoid the dead lock cause the functor can call queueInloop(;)
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) {
    functors[i]();
  }
  callingPendingFunctors_ = false;
}
