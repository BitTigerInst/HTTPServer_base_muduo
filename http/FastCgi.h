#ifndef MUDUO_NET_FAST_CGI_H
#define MUDUO_NET_FAST_CGI_H

#include <muduo/base/Types.h>
#include <muduo/base/noncopyable.h>
#include <muduo/http/FastCgiSpec.h>
#include <vector>

namespace muduo {
namespace net {

class Buffer;

/**
 * this class used for parse fastcgi
 * and pack data
 */

class FastCgi : noncopyable {
 public:
  FastCgi() { requestId_ = 0; }
  ~FastCgi() {}

  void setRequestId(int requestId) { requestId_ = requestId; }

  int getRequestId(Buffer* buffer);

  void StartRequestRecord(Buffer *buffer);

  void Params(Buffer *buffer, string& name, string& value);

  void Params(Buffer *buffer, std::vector<string>& name, std::vector<string>& value);

  void EndRequestRecord(Buffer *buffer);

  string ParseFromPhp(Buffer* buffer);

 private:
  FCGI_Header makeHeader(int type, int request, int contentLength,
                         int paddingLength);

  FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConnection);

  bool makeNameValueBody(string name, int nameLen, std::string value,
                         int valueLen, unsigned char *bodyBuffPtr,
                         int *bodyLenPtr);
  int requestId_;

  static const int PARAMS_BUFF_LEN ;   //环境参数buffer的大小
  static const int CONTENT_BUFF_LEN ;  //内容buffer的大小
};

}  // net
}  // muduo

#endif  // MUDUO_NET_FAST_CGI_H
