#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <muduo/base/Timestamp.h>
#include <muduo/base/noncopyable.h>
#include <muduo/net/EventLoop.h>
#include <map>
#include <vector>

struct pollfd;
namespace muduo {
namespace net {

class Channel;

///
/// IO Multiplexing with poll(2).
///
/// This class doesn't own the Channel objects.
class Poller : noncopyable {
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  Timestamp poll(int timeoutMs, ChannelList* activeChannels);

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  void updateChannel(Channel* channel);

  /// Remove the channel, when it destructs.
  /// Must be called in the loop thread.
  void removeChannel(Channel* channel);

  void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

 private:
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;
  typedef std::map<int, Channel*> ChannelMap;  // fd to Channel

  EventLoop* ownerLoop_;  // POll life equals eventloop?
  PollFdList pollfds_;
  ChannelMap channels_;
};

}  // net
}  // muduo

#endif  // MUDUO_NET_POLLER_H
