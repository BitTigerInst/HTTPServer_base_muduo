
#include <muduo/http/HttpServer.h>

#include <muduo/base/Logging.h>

#include <muduo/http/HttpRequest.h>
#include <muduo/http/HttpResponse.h>
#include <functional>


using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;

namespace muduo
{
namespace net
{
namespace detail
{

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

}
}
}

HttpServer::HttpServer(EventLoop* loop,
             const InetAddress& listenAddr,
             const std::string& name)
        : server_(loop,listenAddr, name),
    httpCallback_(detail::defaultHttpCallback),
    time_out(10)//hard code there
{
  server_.setConnectionCallback(
      std::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, _1, _2, _3));
  loop->runEvery(1.5,std::bind(&HttpServer::onCheckTimer,this));
#ifdef NDEBUG
  dumpConnectionList();
#endif
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
  LOG_WARN << "HttpServer[" << server_.name()
    << "] starts listenning on " << server_.ipPort();
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    HttpContext context;
    context.setLastReceiveTime(Timestamp::now());
    connectionList_.push_back(conn);
    context.setPostion(--connectionList_.end());
    conn->setContext(context);
  }
  else
  {
    LOG_DEBUG << "onConnection  is connected false" ;
    const HttpContext& context = conn->getContext();
    connectionList_.erase(context.getPosition());
  }

#ifdef NDEBUG
  dumpConnectionList();
#endif
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime)
{
  HttpContext* context = conn->getMutableContext();
  ///to update the connectionList
  context->setLastReceiveTime(receiveTime);
  connectionList_.splice(connectionList_.end(),connectionList_,context->getPosition());
  assert(context->getPosition() == --connectionList_.end());
#ifdef NDEBUG
  dumpConnectionList();
#endif

  if (!context->parseRequest(buf, receiveTime))
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll())
  {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  const std::string& connection = req.getHeader("Connection");
  bool close = connection == "close" ||
    (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection())
  {
    conn->shutdown();
  }
}

void HttpServer::onCheckTimer()
{
#ifdef NDEBUG
  dumpConnectionList();
#endif
  Timestamp now = Timestamp::now();

  for(WeakConnectionList::iterator it = connectionList_.begin();
    it != connectionList_.end();)
  {
    TcpConnectionPtr conn = it->lock();
    if(conn)
    {
      HttpContext* context = conn->getMutableContext();
      double age = timeDifference(now,context->getLastReceiveTime());
      if(age > time_out)//already time_out
      {
        if(conn->connected())
        {
          conn->shutdown();
          LOG_INFO << "shutting down" << conn->name();
          conn->forceCloseWithDelay(3.5);  // > round trip of the whole Internet.
        }
      }
      else if(age < 0)
      {
        LOG_WARN << "Time jump";
        context->setLastReceiveTime(now);
      }
      else
      {
        break;//already kick out all timeout connection
      }
      ++it;
    }
    else
    {
      LOG_WARN << "Expired";
      it = connectionList_.erase(it);
    }
  }

}

void HttpServer::dumpConnectionList() const
{
  LOG_INFO << "size = " << connectionList_.size();

  for (WeakConnectionList::const_iterator it = connectionList_.begin();
      it != connectionList_.end(); ++it)
  {
    TcpConnectionPtr conn = it->lock();
    if (conn)
    {
      printf("conn %p\n", conn.get());
      const HttpContext context = conn->getContext();
      printf("    time %s\n", context.getLastReceiveTime().toString().c_str());
    }
    else
    {

      printf("expired\n");
    }
  }
}