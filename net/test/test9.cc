#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <stdio.h>

void onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp receiveTime)
{
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

  conn->send(buf->retrieveAsString());
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d\n", getpid());

  muduo::net::InetAddress listenAddr(9981);
  muduo::net::EventLoop loop;

  muduo::net::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();

  loop.loop();
}
