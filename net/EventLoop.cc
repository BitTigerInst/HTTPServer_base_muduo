
#include <muduo/net/EventLoop.h>
#include <muduo/net/Poller.h>
#include <muduo/net/Channel.h>
#include <muduo/base/Logging.h>
#include <muduo/net/TimerQueue.h>

#include <assert.h>
#include <poll.h>

using namespace muduo;
using namespace muduo::net;

// gcc  local thread store
__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(CurrentThread::tid()),
      poller_(new Poller(this)),
      timerQueue_(new TimerQueue(this))
{
  LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
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

  while(!quit_)
  {
    activeChannels_.clear();
    poller_->poll(kPollTimeMs,&activeChannels_);
    for(ChannelList::iterator it = activeChannels_.begin();
      it!= activeChannels_.end();++it)
    {
      (*it)->handleEvent();
    }
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}
void EventLoop::quit()
{
  quit_ = true;
  // wakeup();
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback&& cb)
{
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback&& cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, const TimerCallback&& cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::updateChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << CurrentThread::tid();
}