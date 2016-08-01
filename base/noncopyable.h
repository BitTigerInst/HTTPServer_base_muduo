#ifndef MUODUO_NET_NONCOPYABLE_H
#define MUODUO_NET_NONCOPYABLE_H

class noncopyable
{
 protected:
  noncopyable() = default;
  ~noncopyable()= default;

 private:
  noncopyable(const noncopyable&) = delete;
  const noncopyable& operator=(const noncopyable&) = delete;
};


#endif // MUODUO_NET_NONCOPYABLE_H

