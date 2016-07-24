
#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include <muduo/net/Callbacks.h>
#include <muduo/net/TcpConnection.h>

#include <map>
#include <muduo/base/noncopyable.h>
#include <memory>

namespace muduo
{
namespace net
{



class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
 public:

  TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  ~TcpServer();  // force out-line dtor, for scoped_ptr members.

  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  void setThreadNum(int numThreads); 


  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback&& cb)
  { connectionCallback_ = std::move(cb); }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback&& cb)
  { messageCallback_ = std::move(cb); }



  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(const WriteCompleteCallback&& cb)
  { writeCompleteCallback_ = std::move(cb); }

 private:
  /// Not thread safe, but in loop
  void newConnection(Socket&& sockfd, const InetAddress& peerAddr);
  /// Thread safe
  void removeConnection(const TcpConnectionPtr& conn);
  /// Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);
  
  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;  // the acceptor loop
  const std::string name_;
  const std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
  const std::unique_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool started_;
  int nextConnId_;  // always in loop thread
  ConnectionMap connections_;
};

}
}
#endif  // MUDUO_NET_TCPSERVER_H
