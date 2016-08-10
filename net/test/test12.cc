#include <muduo/net/Connector.h>
#include <muduo/net/EventLoop.h>


#include <stdio.h>

muduo::net::EventLoop* g_loop;

void connectCallback(muduo::net::Socket&& sockfd)
{
  printf("connected.\n");
  g_loop->quit();
}

int main(int argc, char* argv[])
{
  muduo::net::EventLoop loop;
  g_loop = &loop;
  muduo::net::InetAddress addr("127.0.0.1", 9000);
  muduo::net::ConnectorPtr connector(new muduo::net::Connector(&loop, addr));
  connector->setNewConnectionCallback(connectCallback);
  connector->start();

  loop.loop();
}
