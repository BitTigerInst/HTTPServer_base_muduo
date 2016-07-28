
#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H
#include <muduo/base/Types.h>
#include <muduo/base/StringPiece.h>
#include <muduo/base/noncopyable.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Socket.h>
#include <muduo/http/HttpContext.h>

#include <memory>

namespace muduo {
namespace net {

class Channel;
class EventLoop;

///
/// TCP connection, for both client and server usage.
///
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  /// Constructs a TcpConnection with a connected sockfd
  ///
  /// User should not create this object.
  TcpConnection(EventLoop* loop, const string& name, Socket&& sockfd,
                const InetAddress& localAddr, const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const string& name() const { return name_; }
  const InetAddress& localAddress() { return localAddr_; }
  const InetAddress& peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }

  // void send(string&& message); // C++11
  void send(const void* message, int len);
  void send(const StringPiece& message);
  // void send(Buffer&& message); // C++11
  void send(Buffer* message);  // this one will swap data
  void shutdown();
  void setTcpNoDelay(bool on);


  void setContext(const HttpContext& context)
  { context_ = context; }

  const HttpContext& getContext() const
  { return context_; }

  HttpContext* getMutableContext()
  { return &context_; }


  // cannot use rvalue
  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

  /// Internal use only.
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

  // called when TcpServer accepts a new connection
  void connectEstablished();  // should be called only once

  // called when TcpServer has removed me from its map
  void connectDestroyed();  // should be called only once

 private:
  enum StateE {
    kConnecting,
    kConnected,
    kDisconnecting,
    kDisconnected,
  };

  void setState(StateE s) { state_ = s; }
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  // void sendInLoop(string&& message);
  void sendInLoop(const StringPiece& message);
  void sendInLoop(const void* message, size_t len);
  void shutdownInLoop();

  EventLoop* loop_;
  string name_;
  StateE state_;  // FIXME: use atomic variable
  // we don't expose those classes to client.
  // std::unique_ptr<Socket> socket_;
  Socket socket_;//not to use const since we will close socket
  const std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;

  CloseCallback closeCallback_;

  Buffer inputBuffer_;
  Buffer outputBuffer_;

  HttpContext context_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // net
}

#endif  // MUDUO_NET_TCPCONNECTION_H
