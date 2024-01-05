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
  if (sockfd_ != kInvalidSockfd)
    socket_api::close(sockfd_);
}

int Socket::bind(const NetAddress &addr)
{
  int r;
  r = socket_api::bind(sockfd_, addr.ip.c_str(), addr.port);
  return r;
}

int Socket::listen(int backlog)
{
  int r;
  r = socket_api::listen(sockfd_, backlog);
  return r;
}

int Socket::connect(const NetAddress &addr)
{
  int r;
  r = socket_api::connect(sockfd_, addr.ip.c_str(), addr.port);
  return r;
}
SockfdAddressPair Socket::accept()
{
  int r;
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
  int r;
  r = socket_api::send(sockfd_, static_cast<const char *>(data), len);
  return r;
}
int Socket::recv(void *data, int len)
{
  int r;
  r = socket_api::recv(sockfd_, static_cast<char *>(data), len);
  return r;
}

NAMESPACE_END(net)
LY_NAMESPACE_END
