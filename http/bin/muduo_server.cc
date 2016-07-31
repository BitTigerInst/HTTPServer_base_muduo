#include <muduo/base/Logging.h>
#include <muduo/http/HttpRequest.h>
#include <muduo/http/HttpResponse.h>
#include <muduo/http/HttpServer.h>
#include <muduo/net/EventLoop.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <map>
#include <fcntl.h>
#include <unistd.h>

/**

  TODO:
  - static Content
  - php-
  - http://blog.sina.com.cn/s/blog_4d8cf3140101pa8c.html
  - http://www.cnblogs.com/skynet/p/4173450.html
  -

 */
using namespace muduo;
using namespace muduo::net;

const std::string WEB_PATH = "../web";
const int NUM_THREAD = 1;
bool benchmark = false;

void get_filetype(const string& filename, string& filetype) {
  if (filename.find(".html"))
    filetype = "text/html";
  else if (filename.find(".jpg"))
    filetype = "image/jpeg";
  else if (filename.find(".gif"))
    filetype = "image/gif";
  else if (filename.find(".mp4"))
    filetype = "video/mp4";
  else
    filetype = "text/plain";
}
/// ssize_t pread(int fd, void *buf, size_t count, off_t offset);
void server_static(HttpResponse* resp, const string& reqpath) {
  struct stat sbuf;
  string filename = WEB_PATH + reqpath;

  LOG_DEBUG << filename ;
  if (stat(filename.c_str(), &sbuf) < 0) {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
    return;
  }
  if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
    resp->setStatusCode(HttpResponse::k403Forbidden);
    resp->setStatusMessage("Forbidden");
    resp->setCloseConnection(true);
    return;
  }

  LOG_DEBUG << filename ;
  int srcfd;
  string filetype;
  size_t filesize = sbuf.st_size;
  get_filetype(filename, filetype);

  srcfd = ::open(filename.c_str(), O_RDONLY, 0);
  assert(srcfd > 0);
  if (srcfd < 0) {  // read file error!!
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Read Error");
    resp->setCloseConnection(true);
    return;
  }

  resp->setBodyFromfile(srcfd, filesize);
  resp->setStatusCode(HttpResponse::k200Ok);
  resp->setStatusMessage("OK");
  resp->setContentType(filetype);
  return;
}



void onRequest(const HttpRequest& req, HttpResponse* resp) {
  std::cout << "Headers " << req.methodString() << " " << req.path()
            << std::endl;
  if (!benchmark) {
    const std::map<std::string, std::string>& headers = req.headers();
    for (std::map<std::string, std::string>::const_iterator it =
             headers.begin();
         it != headers.end(); ++it) {
      std::cout << it->first << ": " << it->second << std::endl;
    }
  }

  if (req.path().find("cgi-bin") == std::string::npos) {  /// server_static
    string reqpath = req.path();
    if(reqpath == "/")reqpath = reqpath + "home.html";
    server_static(resp,reqpath);
  } 
  else 
  {  //

    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/html");
    resp->addHeader("Server", "Muduo");
    std::string now = Timestamp::now().toFormattedString();
    resp->setBody("<html><head><title>This is title</title></head>"
        "<body><h1>still not support cgi-bin</h1>Now is " + now +
        "</body></html>");
  }
}

int main(int argc, char* argv[]) {
   Logger::setLogLevel(Logger::DEBUG);
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "dummy");
  server.setHttpCallback(onRequest);
  server.setThreadNum(NUM_THREAD);
  server.start();
  loop.loop();
}
