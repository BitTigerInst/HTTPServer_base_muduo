
#include <muduo/net/TcpServer.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Acceptor.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/net/EventLoopThreadPool.h>

#include <functional> //bind

#include <stdio.h>  // snprintf

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;

TcpServer::TcpServer(EventLoop* loop, 
      const InetAddress& listenAddr,
      const std::string& nameArg)
  : loop_(CHECK_NOTNULL(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr)),
    threadPool_(new EventLoopThreadPool(loop)),
    started_(false),
    nextConnId_(1)
{
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::newConnection, 
                this, _1, _2));
}

TcpServer::~TcpServer()
{

}

void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}



void TcpServer::start()
{
  if (!started_)
  {
    started_ = true;
    threadPool_->start();
  }

  if (!acceptor_->listenning())
  {
    loop_->runInLoop(
        std::bind(&Acceptor::listen,acceptor_.get()));
  }
}

void TcpServer::newConnection(Socket&& sockfd, const InetAddress& peerAddr)
{
  loop_->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof buf, "#%d", nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  LOG_INFO << "TcpServer::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peerAddr.toHostPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd.fd()));

  EventLoop* ioLoop = threadPool_->getNextLoop();
  // FIXME poll with zero timeout to double confirm the new connection
  //TcpConnectionPtr conn(
  //   new TcpConnection(loop_, connName, std::move(sockfd), localAddr, peerAddr));
  TcpConnectionPtr conn = std::make_shared<TcpConnection>
            (ioLoop, connName, std::move(sockfd), localAddr, peerAddr);
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection,this,_1));
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
} 

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  // FIXME: unsafe
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  assert(n == 1); (void)n;


  //Channel->handleEvent   life cyle problems!!!
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
}
