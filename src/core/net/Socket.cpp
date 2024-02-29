#include <core/net/Socket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

Socket::Socket(NetProtocol protocol)
{
  domain_ = NetDomain{NetDomain::IPV4};
  protocol_ = protocol;
  if (protocol.isTcp())
    sockfd_ = socket_api::socket_tcp();
  else
    sockfd_ = socket_api::socket_udp();
}
Socket::~Socket()
{
  // if (isValid())
  //   socket_api::close(sockfd_);
}

int Socket::bind(const char *ip, uint16_t port)
{
  int r{-1};
  r = socket_api::bind(sockfd_, ip, port);
  return r;
}
int Socket::bind(const NetAddress &addr)
{
  return this->bind(addr.ip.c_str(), addr.port);
}
int Socket::listen(int backlog)
{
  int r{-1};
  r = socket_api::listen(sockfd_, backlog);
  return r;
}
int Socket::connect(const char *ip, uint16_t port, int msec)
{
  int r{-1};
  if (msec > 0)
    socket_api::set_blocking(sockfd_, msec);
  r = socket_api::connect(sockfd_, ip, port);
  if (msec > 0)
    socket_api::set_nonblocking(sockfd_);
  return r;
}
int Socket::connect(const NetAddress &addr, std::chrono::milliseconds msec)
{
  return this->connect(addr.ip.c_str(), addr.port, msec.count());
}
SockfdAddressPair Socket::accept()
{
  sockfd_t r;
  NetAddress addr;
  char ip[30];
  r = socket_api::accept(sockfd_, ip, &addr.port);
  addr.ip = ip;
  return {r, addr};
}

int Socket::close()
{
  int r = socket_api::close(sockfd_);
  if (r >= 0)
    sockfd_ = kInvalidSockfd;
  return r;
}

int Socket::send(const void *data, int len)
{
  int r{-1};
  r = socket_api::send(sockfd_, static_cast<const char *>(data), len);
  return r;
}
int Socket::recv(void *data, int len)
{
  int r{-1};
  r = socket_api::recv(sockfd_, static_cast<char *>(data), len);
  return r;
}

NAMESPACE_END(net)
LY_NAMESPACE_END
