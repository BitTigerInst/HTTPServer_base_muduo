
#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>

#include <poll.h>
#include <sstream>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fdArg)
    : loop_(loop),
      fd_(fdArg),
      events_(0),
      revents_(0),
      index_(-1),
      eventHandling_(false) {}

Channel::~Channel() {
  // to prevent dtor

  LOG_DEBUG << "Channel Close" ;
  //what will happen if handleEvent not done,but Channel is destroyed???
  assert(!eventHandling_);
  
}

void Channel::update() {
  // update_channel
  loop_->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
  LOG_DEBUG << "POLLRDHUP " << (revents_ & POLLRDHUP) << " " << fd_;
  LOG_DEBUG << "POLLPRI " << (revents_ & POLLPRI);
  LOG_DEBUG << "POLLIN " << (revents_ & POLLIN);
  LOG_DEBUG << "POLLHUP " << (revents_ & POLLHUP);
  eventHandling_ = true;

  if (revents_ & POLLNVAL) {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
  }

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG_WARN << "Channel::handle_event() POLLHUP";
    if (closeCallback_) closeCallback_();
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_) readCallback_(receiveTime);
  }
  if (revents_ & POLLOUT) {
    if (writeCallback_) writeCallback_();
  }

  eventHandling_ = false;
}