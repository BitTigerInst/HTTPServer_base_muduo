

#include <muduo/net/Buffer.h>
#include <muduo/http/HttpContext.h>
#include <muduo/base/Logging.h>

using namespace muduo;
using namespace muduo::net;

bool HttpContext::processRequestLine(const char* begin, const char* end)
{
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.setMethod(start, space))
  {
    start = space+1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      const char* question = std::find(start, space, '?');
      if (question != space)
      {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      }
      else
      {
        request_.setPath(start, space);
      }
      start = space+1;
      succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
      if (succeed)
      {
        if (*(end-1) == '1')
        {
          request_.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end-1) == '0')
        {
          request_.setVersion(HttpRequest::kHttp10);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

// return false if any error
bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{
  bool ok = true;
  bool hasMore = true;
  int body_lenth;

  //LOG_DEBUG << buf->retrieveAllAsString();
  while (hasMore)
  {
    if (state_ == kExpectRequestLine)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        ok = processRequestLine(buf->peek(), crlf);
        if (ok)
        {
          request_.setReceiveTime(receiveTime);
          buf->retrieveUntil(crlf + 2);
          state_ = kExpectHeaders;
        }
        else
        {
          hasMore = false;
        }
      }
      else
      {
        hasMore = false;
      }
    }
    else if (state_ == kExpectHeaders)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf)
        {
          request_.addHeader(buf->peek(), colon, crlf);
        }
        else
        {
          // empty line, end of header   cannot receive /r/n bug
          // FIXME:
          if(request_.getHeader("Content-Length").size() == 0)
          {
            state_ = kGotAll;
            hasMore = false;
          }
          else
          {
            body_lenth = stoi(request_.getHeader("Content-Length"));
            state_ = kExpectBody;
          }

        }
        //loop retrieve last \r\n  
        buf->retrieveUntil(crlf + 2);
      }
      else
      {
        hasMore = false;
      }
    }
    else if (state_ == kExpectBody)
    {
      //FIXME: The Max body lenth
      LOG_DEBUG << "buf->readableBytes(): " << buf->readableBytes();
      LOG_DEBUG << "body_lenth: " << body_lenth;
      /// if not quit for next more data
      if(buf->readableBytes() >= static_cast<size_t>(body_lenth))//has /r/n before body
      {
         
        //buf->retrieve(2);// /r/n
        const char* begin = buf->peek(); 
        const char* end = begin + static_cast<size_t>(body_lenth);
        request_.setBody(begin,end);
        state_ = kGotAll;
        hasMore = false;
      }
      else
      {
        hasMore = false;
      }
    }
  }
  return ok;
}
