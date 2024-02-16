#include <core/net/tcp/TcpSocket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
int TcpSocket::setNoDelay(int on) {
  int ret{-1};
#ifdef TCP_NODELAY
  ret = socket_api::set_keep_alive(sockfd_, on);
#endif
  return ret;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
