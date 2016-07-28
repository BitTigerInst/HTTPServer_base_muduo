
#include <muduo/net/TcpConnection.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>


#include <errno.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;
TcpConnection::TcpConnection(EventLoop* loop, const string& nameArg,
                             Socket&& sockfd, const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(nameArg),
      state_(kConnecting),
      socket_(std::move(sockfd)),
      channel_(new Channel(loop, socket_.fd())),
      localAddr_(localAddr),
      peerAddr_(peerAddr) {
  LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
            << " fd=" << socket_.fd();
  channel_->setReadCallback(
    std::bind(&TcpConnection::handleRead, this,_1));
  channel_->setWriteCallback(
    std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(
    std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(
    std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd();
}



void TcpConnection::send(const void* data, int len)
{
  send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message)
{
  if (state_ == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(message);
    }
    else
    {
      loop_->runInLoop(
          std::bind(
            static_cast<void(TcpConnection::*)(const StringPiece&)>
            (&TcpConnection::sendInLoop),
                      this,     // FIXME   
                      message.as_string()));//will copy
                    //std::forward<string>(message)));
    }
  }
}
// FIXME efficiency!!!
void TcpConnection::send(Buffer* buf)
{
  if (state_ == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else
    {
      loop_->runInLoop(
          std::bind(
             static_cast<void(TcpConnection::*)(const StringPiece&)>
                      (&TcpConnection::sendInLoop),
                      this,     // FIXME
                      buf->retrieveAllAsString()));
                    //std::forward<string>(message)));
    }
  }
}

void TcpConnection::sendInLoop(const StringPiece& message)
{
  sendInLoop(message.data(), message.size());
}
void TcpConnection::sendInLoop(const void* data, size_t len)
{
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == kDisconnected)
  {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  // if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = ::write(channel_->fd(), data, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if(remaining == 0 && writeCompleteCallback_)
      {
        loop_->queueInLoop(
          std::bind(writeCompleteCallback_,shared_from_this()));
      }
      else
      {
        LOG_TRACE << "I am going to write more data";
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_SYSERR << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
        {
          faultError = true;
        }
      }
    }
  }
  /**
  
    TODO:
    - highter water callback!!
   */
  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }

}



void TcpConnection::shutdown()
{
  // FIXME: use compare and swap
  if (state_ == kConnected)
  {
    setState(kDisconnecting);
    // FIXME: shared_from_this()?
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
  {
    // we are not writing
    socket_.shutdownWrite();
  }
}

void TcpConnection::setTcpNoDelay(bool on)
{
  socket_.setTcpNoDelay(on);
}


void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnected || state_ == kDisconnecting);
  setState(kDisconnected);
  channel_->disableAll();
  connectionCallback_(shared_from_this());

  loop_->removeChannel(channel_.get());
}



void TcpConnection::handleRead(Timestamp receiveTime)
{
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = ::write(channel_->fd(),
                        outputBuffer_.peek(),
                        outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();

        if(writeCompleteCallback_){
          loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
        }

        if (state_ == kDisconnecting) {
          shutdownInLoop();
        }
      } else {
        LOG_TRACE << "I am going to write more data";
      }
    } else {
      LOG_SYSERR << "TcpConnection::handleWrite";
    }
  } else {
    LOG_TRACE << "Connection is down, no more writing";
  }
}


void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpConnection::handleClose state = " << state_;
  assert(state_ == kConnected || state_ == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  channel_->disableAll();
  // must be the last line
  closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
