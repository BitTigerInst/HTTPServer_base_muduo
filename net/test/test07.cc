#include <muduo/net/Acceptor.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/net/Socket.h>
#include <stdio.h>

void newConnection(muduo::net::Socket&& sockfd, const muduo::net::InetAddress& peerAddr)
{
  muduo::net::Socket tsockfd = std::move(sockfd);
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(tsockfd.fd(), "How are you?\n", 13);
  //muduo::net::sockets::close(sockfd);
}
void newConnection2(muduo::net::Socket&& sockfd, const muduo::net::InetAddress& peerAddr)
{
  muduo::net::Socket tsockfd = std::move(sockfd);
  printf("newConnection2(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(tsockfd.fd(), "thank you!\n", 11);
  //muduo::net::sockets::close(sockfd);
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  muduo::net::InetAddress listenAddr(9981);
  muduo::net::EventLoop loop;

  muduo::net::InetAddress listenAddr2(9982);

  muduo::net::Acceptor acceptor(&loop, listenAddr);
  acceptor.setNewConnectionCallback(newConnection);
  acceptor.listen();

  muduo::net::Acceptor acceptor2(&loop, listenAddr2);
  acceptor2.setNewConnectionCallback(newConnection2);
  acceptor2.listen();

  loop.loop();
}
