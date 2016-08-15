
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
    : state_(kExpectRequestLine),
      cgiRequestId_(0)
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

  void setCgiRequestId(int id)
  { cgiRequestId_ = id; }

  int getCgiRequestId() const
  { return cgiRequestId_; }

  void setClose(bool close)
  { close_ = close ;}

  bool getClose() const
  { return close_; }

 private:
  bool processRequestLine(const char* begin, const char* end);

  HttpRequestParseState state_;
  HttpRequest request_;

  //member to kick out idle connection
  Timestamp lastReceiveTime_;
  Index position_;

  //the requesId for the fast-cgi
  int cgiRequestId_; //0 represent no fastcgi

  bool close_;
  
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPCONTEXT_H
