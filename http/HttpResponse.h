
#ifndef MUDUO_NET_HTTP_HTTPRESPONSE_H
#define MUDUO_NET_HTTP_HTTPRESPONSE_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>
#include <unistd.h>
#include <muduo/base/StringPiece.h>
#include <vector>
#include <assert.h>
#include <map>

namespace muduo
{
namespace net
{

class Buffer;
class HttpResponse : public copyable
{
 public:
  enum HttpStatusCode
  {
    kUnknown,
    k200Ok = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
    k403Forbidden = 403
  };

  explicit HttpResponse(bool close)
    : statusCode_(kUnknown),
      closeConnection_(close)
  {
  }

  void setStatusCode(HttpStatusCode code)
  { statusCode_ = code; }

  void setStatusMessage(const string& message)
  { statusMessage_ = message; }

  void setCloseConnection(bool on)
  { closeConnection_ = on; }

  bool closeConnection() const
  { return closeConnection_; }

  void setContentType(const string& contentType)
  { addHeader("Content-Type", contentType); }

  // FIXME: replace string with StringPiece
  void addHeader(const string& key, const string& value)
  { headers_[key] = value; }

  void setBody(const StringPiece& size)
  { setBody(body_.data(),body_.size());}

  void setBody(const char* data,size_t len)
  {
    body_.resize(len);
    std::copy(data,data+len,body_.data());
  }

  void setBodyFromfile(int srcfd,size_t size)
  {
    body_.resize(size);
    //ssize_t pread(int fd, void *buf, size_t count, off_t offset)
    ssize_t rc = ::pread(srcfd,body_.data(),size,0);
    assert(static_cast<size_t>(rc) == size);(void)rc;
  }

  
  void appendToBuffer(Buffer* output) const;

 private:
  std::map<string, string> headers_;
  HttpStatusCode statusCode_;
  // FIXME: add http version
  string statusMessage_;
  bool closeConnection_;
  std::vector<char> body_;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPRESPONSE_H
