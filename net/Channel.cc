
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
    : loop_(loop), fd_(fdArg), events_(0), revents_(0), index_(-1) {}

void Channel::update() {
  // update_channel
  loop_->updateChannel(this);
}

void Channel::handleEvent() {
  if (revents_ & POLLNVAL) {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_) readCallback_();
  }
  if (revents_ & POLLOUT) {
    if (writeCallback_) writeCallback_();
  }
}