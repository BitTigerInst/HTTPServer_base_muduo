#ifndef MUDUO_NET_HTTP_HTTPSERVER_H
#define MUDUO_NET_HTTP_HTTPSERVER_H

#include <muduo/base/noncopyable.h>
#include <muduo/http/FastCgi.h>
#include <muduo/http/HttpContext.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

namespace muduo {
namespace net {

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable {
 public:
  typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, const InetAddress& listenAddr,
             const std::string& name, const int expiration,
             const InetAddress& connectAddr);

  ~HttpServer();  // force out-line dtor, for scoped_ptr members.

  EventLoop* getLoop() const { return server_.getLoop(); }

  /// Not thread safe, callback be registered before calling start().
  void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  /// Check expire connection and kick out
  void onCheckTimer();
  void dumpConnectionList() const;

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&,
                 HttpContext* context);

  void onCGIConnection(const TcpConnectionPtr& conn);
  void onCGIMessage(const TcpConnectionPtr& conn, Buffer* buf,
                    Timestamp receiveTime);

  void server_cgi(const HttpRequest& req, int reqid,
                  const TcpConnectionPtr& conn);

  TcpServer server_;
  HttpCallback httpCallback_;

  // for kick out the idle connection
  const int time_out;  // expiration time
  WeakConnectionList connectionList_;

  // for php fast-cgi support;
  TcpClient client_;
  TcpConnectionPtr CGIConn_;

  ///[requestId : weakconnectionPtr]  to store connection to reqest cgi;
  AtomicInt32 ReqCGIId_;
  typedef std::map<int, WeakTcpConnectionPtr> connectionMap;
  /// current in one thread , do not use mutex
  connectionMap ReqCGIConnMap_;

  static const std::string WEB_PATH;
};
}
}

#endif  // MUDUO_NET_HTTP_HTTPSERVER_H
