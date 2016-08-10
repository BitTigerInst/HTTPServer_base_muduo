#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>
#include <muduo/base/CurrentThread.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;

void threadFunc()
{
  g_loop->loop();
}

int main()
{
  EventLoop loop;
  g_loop = &loop;
  Thread t(threadFunc);
  t.start();
  t.join();
}


