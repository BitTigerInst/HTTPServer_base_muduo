#include <muduo/base/Logging.h>
#include <muduo/http/HttpRequest.h>
#include <muduo/http/HttpResponse.h>
#include <muduo/http/HttpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/http/StaticFile.h>

#include <sys/types.h>
#include <iostream>
#include <map>


/**

  TODO:
  - static Content
  - php-
  - http://blog.sina.com.cn/s/blog_4d8cf3140101pa8c.html
  - http://www.cnblogs.com/skynet/p/4173450.html
  - https://github.com/jaykizhou/php-server/
  -

 */
using namespace muduo;
using namespace muduo::net;

const std::string WEB_PATH = "../web";
const int NUM_THREAD = 5;
bool benchmark = false;

void get_filetype(const string& filename, string& filetype) {
  if (filename.find(".html") != std::string::npos)
    filetype = "text/html";
  else if (filename.find(".jpg")!= std::string::npos)
    filetype = "image/jpeg";
  else if (filename.find(".gif")!= std::string::npos)
    filetype = "image/gif";
  else if (filename.find(".mp4")!= std::string::npos)
    filetype = "video/mp4";
  else
    filetype = "text/plain";
}
/// ssize_t pread(int fd, void *buf, size_t count, off_t offset);
void server_static(HttpResponse* resp, const string& reqpath) {
  string filename = WEB_PATH + reqpath;

  
  StaticFile file(filename);
  file.openFile();
  if (file.getStatus() == StaticFile::NotFound ||
      file.getStatus() == StaticFile::ReadError) {

    LOG_DEBUG << filename << "problem";
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setContentType("text/html");
    resp->addHeader("Server", "Muduo");
    resp->setBody("<html><head><title>This is title</title></head>"
        "<body><h1>404 Not Found</h1></html>");
    resp->setCloseConnection(true);
    return;
  }
  if (file.getStatus() == StaticFile::Forbidden) {
    resp->setStatusCode(HttpResponse::k403Forbidden);
    resp->setStatusMessage("Forbidden");
    resp->setCloseConnection(true);
    return;
  }

  
  string filetype;
  get_filetype(filename, filetype);

  LOG_DEBUG << filename  << ":::" << filetype;

  resp->setBody(file.readContent());
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

    std::cout << req.body() << std::endl;
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
  HttpServer server(&loop, InetAddress(8000), "muduo_http");
  server.setHttpCallback(onRequest);
  server.setThreadNum(NUM_THREAD);
  server.start();
  loop.loop();
}
