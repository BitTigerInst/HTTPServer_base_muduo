

#include <muduo/http/FastCgi.h>
#include <muduo/net/Buffer.h>

using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::cgi;

const int FastCgi::PARAMS_BUFF_LEN = 1024 ;
const int FastCgi::CONTENT_BUFF_LEN = 1024 ;

FCGI_Header FastCgi::makeHeader(int type, int requestId, int contentLength,
                                int paddingLength) {
  FCGI_Header header;

  header.version = FCGI_VERSION_1;

  header.type = (unsigned char)type;

  header.requestIdB1 =
      (unsigned char)((requestId >> 8) & 0xff);  //用连个字段保存请求ID
  header.requestIdB0 = (unsigned char)(requestId & 0xff);

  header.contentLengthB1 =
      (unsigned char)((contentLength >> 8) & 0xff);  //用俩个字段保存内容长度
  header.contentLengthB0 = (unsigned char)(contentLength & 0xff);

  header.paddingLength = (unsigned char)paddingLength;  //填充字节的长度

  header.reserved = 0;  //保留字节赋为0

  return header;
}

FCGI_BeginRequestBody FastCgi::makeBeginRequestBody(int role,
                                                    int keepConnection) {
  FCGI_BeginRequestBody body;

  body.roleB1 = (unsigned char)((role >> 8) &
                                0xff);  //俩个字节保存我们期望php-fpm扮演的角色
  body.roleB0 = (unsigned char)(role & 0xff);

  body.flags = (unsigned char)((keepConnection) ? FCGI_KEEP_CONN
                                                : 0);  //大于0常连接，否则短连接

  bzero(&body.reserved, sizeof(body.reserved));

  return body;
}

bool FastCgi::makeNameValueBody(std::string name, int nameLen,
                                std::string value, int valueLen,
                                unsigned char *bodyBuffPtr, int *bodyLenPtr) {
  unsigned char *startBodyBuffPtr = bodyBuffPtr;  //记录body的开始位置

  if (nameLen < 128)  //如果nameLen长度小于128字节
  {
    *bodyBuffPtr++ = (unsigned char)nameLen;  // nameLen用一个字节保存
  } else {
    // nameLen用4个字节保存
    *bodyBuffPtr++ = (unsigned char)((nameLen >> 24) | 0x80);
    *bodyBuffPtr++ = (unsigned char)(nameLen >> 16);
    *bodyBuffPtr++ = (unsigned char)(nameLen >> 8);
    *bodyBuffPtr++ = (unsigned char)nameLen;
  }

  if (valueLen < 128)  // valueLen小于128就用一个字节保存
  {
    *bodyBuffPtr++ = (unsigned char)valueLen;
  } else {
    // valueLen用4个字节保存

    *bodyBuffPtr++ = (unsigned char)((valueLen >> 24) | 0x80);
    *bodyBuffPtr++ = (unsigned char)(valueLen >> 16);
    *bodyBuffPtr++ = (unsigned char)(valueLen >> 8);
    *bodyBuffPtr++ = (unsigned char)valueLen;
  }

  //将name中的字节逐一加入body的buffer中
  for (auto ch : name) {
    *bodyBuffPtr++ = ch;
  }

  //将value中的值逐一加入body的buffer中
  for (auto ch : value) {
    *bodyBuffPtr++ = ch;
  }

  //计算出body的长度
  *bodyLenPtr = bodyBuffPtr - startBodyBuffPtr;
  return true;
}

void FastCgi::StartRequestRecord(Buffer *buffer) {
  FCGI_BeginRequestRecord beginRecord;

  beginRecord.header =
      makeHeader(FCGI_BEGIN_REQUEST, requestId_, sizeof(beginRecord.body), 0);
  beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 1);  // keep
                                                               // connetion;

  buffer->append(reinterpret_cast<char *>(&beginRecord), sizeof beginRecord);
}

void FastCgi::Params(Buffer *buffer, string name, string value) {
  unsigned char bodyBuff[PARAMS_BUFF_LEN];

  bzero(bodyBuff, sizeof(bodyBuff));

  int bodyLen;  //保存body的长度

  //生成PARAMS参数内容的body
  makeNameValueBody(name, name.size(), value, value.size(), bodyBuff, &bodyLen);

  FCGI_Header nameValueHeader;
  nameValueHeader = makeHeader(FCGI_PARAMS, requestId_, bodyLen, 0);

  assert(sizeof nameValueHeader == FCGI_HEADER_LEN);
  buffer->append(reinterpret_cast<char *>(&nameValueHeader), sizeof nameValueHeader);
  buffer->append(reinterpret_cast<char *>(bodyBuff), bodyLen);
}



void FastCgi::EndRequestRecord(Buffer* buffer)
{
    FCGI_Header endHeader;

    endHeader = makeHeader(FCGI_PARAMS,requestId_,0,0);

    buffer->append(reinterpret_cast<char *>(&endHeader),sizeof endHeader);
}