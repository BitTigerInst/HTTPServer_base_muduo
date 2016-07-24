

#include <muduo/net/Socket.h>

#include <muduo/net/InetAddress.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/base/Logging.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // bzero

using namespace muduo;
using namespace muduo::net;





Socket::~Socket()
{
  LOG_DEBUG << "close Socket " << sockfd_ ;
  if(sockfd_!=-1){
    sockets::close(sockfd_);
  }
}

Socket::Socket(Socket&& sourcefd)
{
  LOG_TRACE << "move cpy Socket " << sockfd_ ;
  sockfd_ = sourcefd.sockfd_;
  sourcefd.sockfd_ = -1;
}
Socket& Socket:: operator=(Socket&& sourcefd)
{
  LOG_TRACE << "move assgin Socket " << sockfd_ ;
  sockfd_ = sourcefd.sockfd_;
  sourcefd.sockfd_ = -1;
  return *this;
}



void Socket::bindAddress(const InetAddress& addr)
{
  sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen()
{
  sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::accept(sockfd_, &addr);
  if (connfd >= 0)
  {
    peeraddr->setSockAddrInet(addr);
  }
  return connfd;
}

void Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof optval);
  // FIXME CHECK
}

void Socket::shutdownWrite()
{
  sockets::shutdownWrite(sockfd_);
}


void Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, sizeof optval);
  // FIXME CHECK
}