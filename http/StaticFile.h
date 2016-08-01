#ifndef MUDUO_HTTP_STATIC_FILE_H
#define MUDUO_HTTP_STATIC_FILE_H

#include <muduo/base/Types.h>
#include <muduo/base/noncopyable.h>
#include <sys/stat.h>
#include <vector>

namespace muduo {
namespace net {

class StaticFile : noncopyable {
 public:
  enum FileStatusCode {OK, NotFound, Forbidden, ReadError };

  StaticFile(const string& filename);

  ~StaticFile();  // destructor , so no move

  std::vector<char> readContent();

  int openFile();

  FileStatusCode getStatus() { return status_; }

 private:
  const string filename_;
  int fd_;
  struct stat sbuf_;
  FileStatusCode status_;
};

}  // net
}  // muduo

#endif  // MUDUO_HTTP_STATIC_FILE_H
