
#include <muduo/net/TcpServer.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Acceptor.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>

#include <functional> //bind

#include <stdio.h>  // snprintf

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(CHECK_NOTNULL(loop)),
    name_(listenAddr.toHostPort()),
    acceptor_(new Acceptor(loop, listenAddr)),
    started_(false),
    nextConnId_(1)
{
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::newConnection, 
                this, std::placeholders::_1, 
                      std::placeholders::_2));
}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
  if (!started_)
  {
    started_ = true;
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
  // FIXME poll with zero timeout to double confirm the new connection
  //TcpConnectionPtr conn(
  //   new TcpConnection(loop_, connName, std::move(sockfd), localAddr, peerAddr));
  TcpConnectionPtr conn = std::make_shared<TcpConnection>
            (loop_, connName, std::move(sockfd), localAddr, peerAddr);
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
  conn->connectEstablished();
} 

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnection [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  assert(n == 1); (void)n;
  loop_->queueInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
  //this will cause ~Channel()  before Channel::handleEvent 
  //loop_->runInLoop(
      //std::bind(&TcpConnection::connectDestroyed, conn));
}