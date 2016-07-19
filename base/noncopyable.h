#ifndef MUODUO_NET_NONCOPYABLE_H
#define MUODUO_NET_NONCOPYABLE_H

class noncopyable
{
 protected:
  noncopyable() {}

 private:
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
};


#endif // MUODUO_NET_NONCOPYABLE_H

