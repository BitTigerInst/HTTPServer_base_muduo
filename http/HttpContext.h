
#ifndef MUDUO_NET_HTTP_HTTPCONTEXT_H
#define MUDUO_NET_HTTP_HTTPCONTEXT_H

#include <muduo/base/copyable.h>

#include <muduo/http/HttpRequest.h>
#include <list>
#include <memory>

namespace muduo
{
namespace net
{

class Buffer;
class TcpConnection;

typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;
typedef std::list<WeakTcpConnectionPtr> WeakConnectionList;
typedef WeakConnectionList::iterator Index;

class HttpContext : public copyable
{
 public:
  enum HttpRequestParseState
  {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HttpContext()
    : state_(kExpectRequestLine)
  {
  }

  // default copy-ctor, dtor and assignment are fine

  // return false if any error
  bool parseRequest(Buffer* buf, Timestamp receiveTime);

  bool gotAll() const
  { return state_ == kGotAll; }

  void reset()
  {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest& request() const
  { return request_; }

  HttpRequest& request()
  { return request_; }

  Timestamp& getLastReceiveTime()
  { return lastReceiveTime_; }

  const Timestamp& getLastReceiveTime()const
  { return lastReceiveTime_; }

  Index& getPosition()
  { return position_; }

  const Index& getPosition()const
  { return position_; }


  void setLastReceiveTime(Timestamp receiveTime)
  { lastReceiveTime_ = receiveTime; }

  void setPostion(Index index)
  { position_ = index; }

 private:
  bool processRequestLine(const char* begin, const char* end);

  HttpRequestParseState state_;
  HttpRequest request_;

  //member to kick out idle connection
  Timestamp lastReceiveTime_;
  Index position_;
  
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPCONTEXT_H
