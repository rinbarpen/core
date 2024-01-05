#pragma once
#include <core/net/SocketUtil.h>

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(net)

class Pipe
{
public:
  Pipe() {}
  ~Pipe() { close(); }

  bool create();
  bool create(sockfd_t in, sockfd_t out);
  int write(void *buf, int len);
  int read(void *buf, int len);
  bool close();

  sockfd_t readFd() const { return fds_[0]; }
  sockfd_t writeFd() const { return fds_[1]; }

private:
  sockfd_t fds_[2];
};

NAMESPACE_END(net)

LY_NAMESPACE_END
