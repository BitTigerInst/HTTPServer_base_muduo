#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <stdio.h>

std::string message;

void onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
    conn->send(message);
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onWriteComplete(const muduo::net::TcpConnectionPtr& conn)
{
  conn->send(message);
}

void onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp receiveTime)
{
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

  buf->retrieveAll();
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d\n", getpid());

  std::string line;
  for (int i = 33; i < 127; ++i)
  {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127-33; ++i)
  {
    message += line.substr(i, 72) + '\n';
  }

  muduo::net::InetAddress listenAddr(9981);
  muduo::net::EventLoop loop;

  muduo::net::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.setWriteCompleteCallback(onWriteComplete);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();

  loop.loop();
}
