#include <fcntl.h>
#include <muduo/base/Logging.h>
#include <muduo/http/StaticFile.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

StaticFile::StaticFile(const string& filename) 
    : filename_(filename), 
    fd_(-1),
    status_(OK) 
{
  if (stat(filename_.c_str(), &sbuf_) < 0) {

    LOG_WARN << filename_ << " not found";
    status_ = NotFound;
  }

  if (!(S_ISREG(sbuf_.st_mode)) || !(S_IRUSR & sbuf_.st_mode)) {
    LOG_WARN << filename_ << " Forbidden";
    status_ = Forbidden;
  }
}

StaticFile::~StaticFile()  // destructor , so no move
{
  LOG_DEBUG << "close file " << fd_;
  ::close(fd_);
}

int StaticFile::openFile() {
  fd_ = ::open(filename_.c_str(), O_RDONLY, 0);
  if (fd_ < 0)
  {
    status_ = ReadError;
    LOG_WARN << "openfile error" << filename_ ;
  } 
    
  return fd_;
}

std::vector<char> StaticFile::readContent() {
  std::vector<char> content;
  size_t len = sbuf_.st_size;
  content.resize(len);
  // ssize_t pread(int fd, void *buf, size_t count, off_t offset)
  ssize_t rc = ::pread(fd_, content.data(), len, 0);
  assert(static_cast<size_t>(rc) == size);
  (void)rc;
  LOG_DEBUG << "read file content len:" << rc;
  return content;
}