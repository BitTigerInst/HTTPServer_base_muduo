
#include <muduo/http/HttpServer.h>

#include <muduo/base/Logging.h>

#include <muduo/http/HttpRequest.h>
#include <muduo/http/HttpResponse.h>
#include <functional>
#include <vector>
#include <muduo/base/Types.h>

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;

namespace muduo {
namespace net {
namespace detail {

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}
}
}
}
/**

  TODO:
  - to move static file from muduo_server to here
  - parse the content of php-fpm
  - CGIconnetion re connect
  - how to keep CGIconnetion alive?

 */


const string HttpServer::WEB_PATH = "/home/xibaohe/ServerCode/chenshuo/muduo_11/muduo/http/web";

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr,
                       const std::string& name, const int expiration,const InetAddress& connectAddr)
    : server_(loop, listenAddr, name),
      httpCallback_(detail::defaultHttpCallback),
      time_out(expiration),  
      client_(loop, connectAddr) 
{
  server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, _1, _2, _3));
  loop->runEvery(1.5, std::bind(&HttpServer::onCheckTimer, this));
#ifdef CONNRM
  dumpConnectionList();
#endif
  client_.setConnectionCallback(
      std::bind(&HttpServer::onCGIConnection, this, _1));
  client_.setMessageCallback(
      std::bind(&HttpServer::onCGIMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer() {}

void HttpServer::start() {
  LOG_WARN << "HttpServer[" << server_.name() << "] starts listenning on "
           << server_.ipPort();
  server_.start();

  client_.connect();
}

void HttpServer::onCGIConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    LOG_INFO << "onConnection(): new connection [" << conn->name().c_str()
             << "] from " << conn->peerAddress().toHostPort().c_str();

    ///to call some function open up the connection
    ///the first package to keep the connection open!!!
    FastCgi fcgi_;
    Buffer buffer;
    fcgi_.StartRequestRecord(&buffer);
    std::vector<string> name;
    std::vector<string> value;
    name.push_back("SCRIPT_FILENAME");
    name.push_back("REQUEST_METHOD");
    value.push_back("/home/xibaohe/ServerCode/chenshuo/muduo_11/muduo/http/web/cgi-bin/info.php");
    value.push_back("GET");
   
    fcgi_.Params(&buffer,name,value);
    fcgi_.EndRequestRecord(&buffer);
    CGIConn_ = conn;
    conn->send(&buffer);   
  } else {
    LOG_ERROR << "onConnection(): connection [" << conn->name().c_str()
              << "] is down";
  }
}
void HttpServer::onCGIMessage(const TcpConnectionPtr& conn, Buffer* buf,
                              Timestamp receiveTime) {
  LOG_DEBUG << "onMessage(): received " << buf->readableBytes()
            << " bytes from connection [" << conn->name().c_str() << "] at"
            << receiveTime.toFormattedString().c_str();
  FastCgi fcgi_;
  while(buf->readableBytes() >= FCGI_HEADER_LEN)
  {
    int requestid = fcgi_.getRequestId(buf);
    string content = fcgi_.ParseFromPhp(buf);
    if(requestid == 0)
    {
      LOG_DEBUG << content ;
    }
    else
    {
      TcpConnectionPtr conn = ReqCGIConnMap_[requestid].lock();
      if(conn)
      {
        HttpContext context = conn->getContext();
        HttpResponse response(context.getClose());
        response.setBody(content);
        response.setStatusCode(HttpResponse::k200Ok);
        response.setStatusMessage("OK");
        response.setContentType("text/html");
        response.addHeader("Server", "Muduo");
        Buffer buf;
        response.appendToBuffer(&buf);
        conn->send(&buf);
      }
      else
      {//remove the invalid connection
        ReqCGIConnMap_.erase(requestid);
      }
    }

  }
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    HttpContext context;
    context.setLastReceiveTime(Timestamp::now());
    connectionList_.push_back(conn);
    context.setPostion(--connectionList_.end());
    conn->setContext(context);
  } else {
    LOG_DEBUG << "onConnection  is connected false";
    const HttpContext& context = conn->getContext();
    connectionList_.erase(context.getPosition());
  }

#ifdef CONNRM
  dumpConnectionList();
#endif
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp receiveTime) {
  HttpContext* context = conn->getMutableContext();
  /// to update the connectionList
  context->setLastReceiveTime(receiveTime);
  connectionList_.splice(connectionList_.end(), connectionList_,
                         context->getPosition());
  assert(context->getPosition() == --connectionList_.end());
#ifdef CONNRM
  dumpConnectionList();
#endif

  if (!context->parseRequest(buf, receiveTime)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll()) {
    onRequest(conn, context->request(),context);
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn,
                           const HttpRequest& req,
                           HttpContext* context) {
  const std::string& connection = req.getHeader("Connection");
  bool close =
      connection == "close" ||
      (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  context->setClose(close);
  HttpResponse response(close);
  if(req.path().find("cgi-bin") == std::string::npos)
  {
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
  }
  else
  {//to php-fpm
    int reqid = context->getCgiRequestId();
    EventLoop* mainLoop = getLoop();
    //to send fastcgi in MainLoop;
    mainLoop->runInLoop(std::bind(&HttpServer::server_cgi,this,req,reqid,conn));
  }
  
  if (response.closeConnection()) {
    conn->shutdown();
  }
}
void HttpServer::server_cgi(const HttpRequest& req,int reqid,const TcpConnectionPtr& conn)
{
    FastCgi fcgi_;
    if(!reqid)
    {// first request fast-cgi
      reqid = ReqCGIId_.incrementAndGet();
      ReqCGIConnMap_[reqid] = conn;/// store weak_ptr in map;
    }
    fcgi_.setRequestId(reqid);
    string filename = WEB_PATH + req.path();

    LOG_DEBUG << "the request php file is " << filename;


    Buffer buffer;
    fcgi_.StartRequestRecord(&buffer);

    std::vector<string> name;
    std::vector<string> value;
    name.push_back("SCRIPT_FILENAME");
    name.push_back("REQUEST_METHOD");
    value.push_back(filename);
    value.push_back(req.methodString());
    fcgi_.Params(&buffer,name,value);
    fcgi_.EndRequestRecord(&buffer);
    if(CGIConn_->connected())
    {
      CGIConn_->send(&buffer);
    }
    else
    {
      LOG_DEBUG << "CGIConnection down!!!" ;
    }
}


void HttpServer::onCheckTimer() {
#ifdef CONNRM
  dumpConnectionList();
#endif
  Timestamp now = Timestamp::now();

  for (WeakConnectionList::iterator it = connectionList_.begin();
       it != connectionList_.end();) {
    TcpConnectionPtr conn = it->lock();
    if (conn) {
      HttpContext* context = conn->getMutableContext();
      double age = timeDifference(now, context->getLastReceiveTime());
      if (age > time_out)  // already time_out
      {
        if (conn->connected()) {
          conn->shutdown();
          LOG_INFO << "shutting down" << conn->name();
          conn->forceCloseWithDelay(
              3.5);  // > round trip of the whole Internet.
        }
      } else if (age < 0) {
        LOG_WARN << "Time jump";
        context->setLastReceiveTime(now);
      } else {
        break;  // already kick out all timeout connection
      }
      ++it;
    } else {
      LOG_WARN << "Expired";
      it = connectionList_.erase(it);
    }
  }
}

void HttpServer::dumpConnectionList() const {
  LOG_INFO << "size = " << connectionList_.size();

  for (WeakConnectionList::const_iterator it = connectionList_.begin();
       it != connectionList_.end(); ++it) {
    TcpConnectionPtr conn = it->lock();
    if (conn) {
      printf("conn %p\n", conn.get());
      const HttpContext context = conn->getContext();
      printf("    time %s\n", context.getLastReceiveTime().toString().c_str());
    } else {
      printf("expired\n");
    }
  }
}
