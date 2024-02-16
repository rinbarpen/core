#pragma once

#include <cstdint>
#include <cstring>
#include <core/util/marcos.h>
#include <core/net/platform.h>
#include <core/net/NetAddress.h>
#include <core/util/logger/Logger.h>


LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

struct NetDomain
{
  enum Type
  {
    NONE = 0,
    IPV4 = AF_INET,
    IPV6 = AF_INET6,
  } type;

  bool isIpv4() const { return type == IPV4; }
  bool isIpv6() const { return type == IPV6; }

  static NetDomain toDomain(std::string name) {
    if (name == "IPV4")
      return {IPV4};
    else if (name == "IPV6")
      return {IPV6};
    return {NONE};
  }
  std::string toString() const {
    switch (type) {
    case IPV4: return "IPV4";
    case IPV6: return "IPV6";
    default: return "NONE";
    }

    return "NONE";
  }
};
struct NetProtocol
{
  enum Type
  {
    NONE = 0,
    UDP = IPPROTO_UDP,
    TCP = IPPROTO_TCP,
  } type;

  NetProtocol() = default;
  NetProtocol(Type _type) : type(_type) {}

  bool isUdp() const { return type == UDP; }
  bool isTcp() const { return type == TCP; }

  static NetProtocol toProcotol(std::string name) {
    if (name == "UDP")
      return {UDP};
    else if (name == "TCP")
      return {TCP};
    return {NONE};
  }
  std::string toString() const {
    switch (type) {
    case UDP: return "UDP";
    case TCP: return "TCP";
    default: return "NONE";
    }

    return "NONE";
  }
};

static constexpr sockfd_t kInvalidSockfd = -1;
// TODO: Add ipv6
namespace socket_api
{
static bool is_valid(sockfd_t fd) {
#ifdef __LINUX__
  return fd > 0;
#elif defined(__WIN__)
  return fd != SOCKET_ERROR;
#endif
}
static sockfd_t socket(int domain, int type, int protocol) {
  return ::socket(domain, type, protocol);
}
static sockfd_t socket_tcp() {
  return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}
static sockfd_t socket_udp() {
  return ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}
static int bind(sockfd_t sockfd, const char *ip, uint16_t port) {
  int r{-1};

  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);

  r = ::inet_pton(AF_INET, ip, &addr.sin_addr);
  if (r < 0) {
    return r;
  }

  r = ::bind(
    sockfd, reinterpret_cast<const struct sockaddr *>(&addr), INET_ADDRSTRLEN);

  return r;
}

static int close(sockfd_t sockfd) {
#ifdef __WIN__
  return ::closesocket(sockfd);
#elif defined(__LINUX__)
  return ::close(sockfd);
#endif
}

/**
 * @brief create a udp socket and bind a (ip, port)
 *        close socket if fail to bind
 *
 * @param ip
 * @param port
 * @return sockfd_t
 */
static sockfd_t socket_udp_bind(const char *ip, uint16_t port) {
  sockfd_t sockfd = socket_udp();
  if (is_valid(sockfd)) return sockfd;

  if (bind(sockfd, ip, port) < 0) {
    close(sockfd);
  }
  return sockfd;
}

static int listen(sockfd_t fd, int backlog) {
  int r{-1};
  r = ::listen(fd, backlog);
  return r;
}

/**
 * @brief create a custom socket and bind a (ip, port), then listen in it
 *        close socket if fail to bind or fail to listen
 *
 * @param domain
 * @param type
 * @param protocol
 * @param ip
 * @param port
 * @param backlog
 * @return sockfd_t
 */
static sockfd_t socket_bind_listen(int domain, int type, int protocol,
  const char *ip, uint16_t port, int backlog) {
  sockfd_t fd = socket(domain, type, protocol);
  if (is_valid(fd)) {
    return fd;
  }

  int r{-1};
  r = bind(fd, ip, port);
  if (r < 0) {
    close(fd);
    return r;
  }
  r = listen(fd, backlog);
  if (r < 0) {
    close(fd);
    return r;
  }

  return fd;
}

static sockfd_t accept(sockfd_t sockfd, char *ip, uint16_t *port) {
  struct sockaddr_storage addr;
  socklen_t len;
  sockfd_t clientfd = ::accept(sockfd, (struct sockaddr *) &addr, &len);
  if (is_valid(clientfd)) return clientfd;

  struct sockaddr_in *addr4 = reinterpret_cast<struct sockaddr_in *>(&addr);
  *port = ::ntohs(addr4->sin_port);
  if (::inet_ntop(AF_INET, &addr4->sin_addr, ip, sizeof(ip)) == nullptr) {
    return kInvalidSockfd;
  }
  return clientfd;
}
static int connect(sockfd_t sockfd, const char *ip, uint16_t port) {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
  int r = ::inet_pton(AF_INET, ip, &addr.sin_addr);
  if (r < 0) { return r; }

  return ::connect(
    sockfd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
}

static int send(sockfd_t sockfd, const char *buf, size_t len, int flags = 0) {
  return ::send(sockfd, buf, len, flags);
}
static int recv(sockfd_t sockfd, char *buf, size_t len, int flags = 0) {
  return ::recv(sockfd, buf, len, flags);
}
static int sendto(sockfd_t fd, const char *buf, uint32_t len, int flags,
  const char *ip, uint16_t port) {
  int r{};
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  r = ::inet_pton(AF_INET, ip, &addr.sin_addr);
  if (r < 0) {
    return r;
  }

  r = ::sendto(fd, buf, len, flags, (struct sockaddr *) &addr, INET_ADDRSTRLEN);
  return r;
}

static NetAddress socket_address(sockfd_t fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if (0 != ::getsockname(fd, (struct sockaddr *) &addr, &len)) {
    return {};
  }

  char ip[INET_ADDRSTRLEN];
  ::inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
  return {ip, ::ntohs(addr.sin_port)};
}
static NetAddress peer_address(sockfd_t fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if (0 != ::getpeername(fd, (struct sockaddr *) &addr, &len)) {
    return {};
  }

  char ip[INET_ADDRSTRLEN];
  ::inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
  return {ip, ::ntohs(addr.sin_port)};
}

static int set_keep_alive(sockfd_t fd, int on = 1) {
  int r{-1};
  r = ::setsockopt(
    fd, SOL_SOCKET, SO_KEEPALIVE, (const char *) (&on), sizeof(on));
  return r;
}
static int set_reuse_address(sockfd_t fd, int on = 1) {
  int r{-1};
  r = ::setsockopt(
    fd, SOL_SOCKET, SO_REUSEADDR, (const char *) (&on), sizeof(on));
  return r;
}
static int set_reuse_port(sockfd_t fd, int on = 1) {
  int r{-1};
#ifdef SO_REUSEPORT
  r = ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char *) &on, sizeof(on));
#endif
  return r;
}
static int set_no_delay(sockfd_t fd, int on = 1) {
  int r{-1};
#ifdef TCP_NODELAY
  r = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &on, sizeof(on));
#endif
  return r;
}
static void set_nonblocking(sockfd_t fd) {
#if defined(__LINUX__)
  int flags = ::fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  ::fcntl(fd, F_SETFL, flags);
#elif defined(__WIN__)
  u_long on = 1;
  ::ioctlsocket(fd, FIONBIO, &on);
#endif
}
static void set_blocking(sockfd_t fd, int write_ms_timeout = 100) {
#if defined(__LINUX__)
  int flags = ::fcntl(fd, F_GETFL);
  flags &= ~O_NONBLOCK;
  ::fcntl(fd, F_SETFL, flags);
#elif defined(__WIN__)
  u_long on = 0;
  ::ioctlsocket(fd, FIONBIO, &on);
#endif
  if (write_ms_timeout > 0) {
#ifdef SO_SNDTIMEO
# if defined(__LINUX__)
    struct timeval tv = {
      write_ms_timeout / 1000, (write_ms_timeout % 1000) * 1000};
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv));
# elif defined(__WIN__)
    unsigned long ms = (unsigned long) write_ms_timeout;
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &ms, sizeof(unsigned long));
# endif
#endif
  }
}

static void set_send_buffer_size(sockfd_t fd, int size) {
  ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(size));
}
static void set_recv_buffer_size(sockfd_t fd, int size) {
  ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(size));
}

}  // namespace socket_api

NAMESPACE_END(net)
LY_NAMESPACE_END
namespace lynet = ly::net;
