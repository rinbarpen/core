#pragma once

#include <core/util/marcos.h>
#include <core/net/SocketUtil.h>
#include <core/net/NetAddress.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

using namespace std::literals;
using SockfdAddressPair = std::pair<sockfd_t, NetAddress>;

struct NetDomain;
struct NetProtocol;

class Socket
{
public:
  SHARED_PTR_USING(Socket, ptr);

  Socket() = default;
  Socket(NetProtocol protocol);
  virtual ~Socket();

    /**
   * \brief warp the socket_api::bind
   * \param[in] addr
   */
  int bind(const NetAddress &addr);
  /**
   * \brief warp the socket_api::listen
   * \param[in] backlog
   */
  int listen(int backlog);
  /**
   * \brief warp the socket_api::connect, use NetAddress instead of the raw data
   * \param[in] addr
   */
  int connect(const NetAddress &addr, std::chrono::milliseconds msec = 0ms);
  /**
   * \brief warp the socket_api::accept
   * \return the accepted socket
   */
  SockfdAddressPair accept();

  int close();

  int send(const void *data, int len);
  int recv(void *data, int len);

  SockfdAddressPair getSockfdAddressPair() const { return {sockfd_, addr_}; }
  sockfd_t getSockfd() const { return sockfd_; }
  bool isIpv6() const { return domain_.isIpv6(); }
  NetDomain getDomain() const { return domain_; }
  std::string getDomainName() const { return domain_.toString(); }
  NetProtocol getProtocol() const { return protocol_; }
  std::string getProtocolName() const { return protocol_.toString(); }
  bool isValid() const { return socket_api::is_valid(sockfd_); }

  std::string getIp() const { return addr_.ip; }
  uint16_t getPort() const { return addr_.port; }
  NetAddress getIpPortPair() const { return addr_; }

  virtual std::string type() const { return "Socket"; }

protected:
  sockfd_t sockfd_{kInvalidSockfd};
  NetAddress addr_;
  NetDomain domain_;
  NetProtocol protocol_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
