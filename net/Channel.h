#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H
#include <muduo/base/Timestamp.h>
#include <muduo/base/noncopyable.h>
#include <functional>

namespace muduo {

namespace net {

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd

class Channel : noncopyable {
 public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(Timestamp)> ReadEventCallback;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handleEvent(Timestamp receiveTime);
  void setReadCallback(const ReadEventCallback&& cb) {
    readCallback_ = std::move(cb);
  }
  void setWriteCallback(const EventCallback&& cb) {
    writeCallback_ = std::move(cb);
  }
  void setErrorCallback(const EventCallback&& cb) {
    errorCallback_ = std::move(cb);
  }
  void setCloseCallback(const EventCallback&& cb) {
    closeCallback_ = std::move(cb);
  }

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool isWriting() const { return events_ & kWriteEvent; }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop* ownerLoop() { return loop_; }

 private:
  void update();

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_;

  bool eventHandling_;

  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback errorCallback_;
  EventCallback closeCallback_;
};

}  // net
}  // muduo

#endif  // MUDUO_NET_CHANNEL_H
