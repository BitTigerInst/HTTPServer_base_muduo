#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <functional>
#include <stdio.h>
#include <muduo/net/TimerId.h>

int cnt = 0;
muduo::net::EventLoop* g_loop;

void printTid()
{
  printf("pid = %d, tid = %d\n", getpid(), muduo::CurrentThread::tid());
  printf("now %s\n", muduo::Timestamp::now().toString().c_str());
}

void print(const char* msg)
{
  printf("msg %s %s\n", muduo::Timestamp::now().toString().c_str(), msg);
  if (++cnt == 20)
  {
    g_loop->quit();
  }
}

muduo::net::TimerId toCancel;
void cancelSelf()
{
  print("cancelSelf()");
  g_loop->cancel(toCancel);
}

int main()
{
  //muduo::Logger::setLogLevel( muduo::Logger::DEBUG);
  printTid();
  muduo::net::EventLoop loop;
  g_loop = &loop;

  print("main");
  loop.runAfter(1, std::bind(print, "once1"));
  loop.runAfter(1.5, std::bind(print, "once1.5"));
  loop.runAfter(2.5, std::bind(print, "once2.5"));
  loop.runAfter(3.5, std::bind(print, "once3.5"));
  muduo::net::TimerId t = loop.runEvery(2, std::bind(print, "every2"));
  loop.runEvery(3, std::bind(print, "every3"));
  loop.runAfter(10, std::bind(&muduo::net::EventLoop::cancel, &loop, t));
  toCancel = loop.runEvery(2, cancelSelf);

  loop.loop();
  print("main loop exits");
  sleep(1);
}