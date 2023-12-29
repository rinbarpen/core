#pragma once

#include <core/util/marcos.h>
#include <core/net/udp/UdpConnection.h>

#include "core/net/Connector.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
// TODO:
class UdpClient
{
public:
  UdpClient();
  virtual ~UdpClient();

  virtual bool start(const char *ip, uint16_t port);
  virtual bool stop();

  LY_NONCOPYABLE(UdpClient);

protected:
  virtual UdpConnection::ptr onConnect(sockfd_t fd);

protected:
  // TcpClientEnv &env_;
  EventLoop *event_loop_;

  std::unique_ptr<Connector> connector_;
  UdpConnection::ptr conn_;

  std::atomic_bool running_{false};
  Mutex::type mutex_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
