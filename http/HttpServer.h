#ifndef MUDUO_NET_HTTP_HTTPSERVER_H
#define MUDUO_NET_HTTP_HTTPSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpClient.h>
#include <muduo/base/noncopyable.h>
#include <muduo/net/EventLoop.h>
#include <muduo/http/HttpContext.h>
#include <muduo/http/FastCgi.h>

namespace muduo
{
namespace net
{

class HttpRequest;
class HttpResponse;


class HttpServer : noncopyable
{
 public:
  typedef std::function<void (const HttpRequest&,
                                HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop,
             const InetAddress& listenAddr,
             const std::string& name,
              const int expiration,
             const InetAddress& connectAddr);

  ~HttpServer();  // force out-line dtor, for scoped_ptr members.

  EventLoop* getLoop() const { return server_.getLoop(); }

  /// Not thread safe, callback be registered before calling start().
  void setHttpCallback(const HttpCallback& cb)
  {
    httpCallback_ = cb;
  }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  /// Check expire connection and kick out
  void onCheckTimer();
  void dumpConnectionList() const;

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);

  void onCGIConnection(const TcpConnectionPtr& conn);
  void onCGIMessage(const TcpConnectionPtr& conn,
                    Buffer* buf,Timestamp receiveTime);

  TcpServer server_;
  HttpCallback httpCallback_;

  //for kick out the idle connection
  const int time_out;//expiration time
  WeakConnectionList connectionList_;

  //for php fast-cgi support;
  TcpClient client_;
  TcpConnectionPtr CGIConn_;
  FastCgi fcgi_;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPSERVER_H
