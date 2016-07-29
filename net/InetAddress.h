
#ifndef MUDUO_NET_INETADDRESS_H
#define MUDUO_NET_INETADDRESS_H

#include <muduo/base/copyable.h>

#include <string>

#include <netinet/in.h>

namespace muduo
{
namespace net {



///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress : copyable
{
 public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  explicit InetAddress(uint16_t port);

  /// Constructs an endpoint with given ip and port.
  /// @c ip should be "1.2.3.4"
  InetAddress(const std::string& ip, uint16_t port);

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
  { }

  std::string toIp() const;
  std::string toIpPort() const;

  std::string toHostPort() const;

  // default copy/assignment are Okay

  const struct sockaddr_in& getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

 private:
  struct sockaddr_in addr_;
};


} // net

}

#endif  // MUDUO_NET_INETADDRESS_H
