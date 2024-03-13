#pragma once

#include <core/net/Connector.h>
#include <core/net/tcp/TcpConnection.h>
#include "core/util/marcos.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class TcpClient
{
public:
  SHARED_PTR_USING(TcpClient, ptr);

  explicit TcpClient(EventLoop *event_loop);
  virtual ~TcpClient();

  virtual bool start(const char *ip, uint16_t port);
  virtual bool stop();

  virtual std::string type() const { return "TcpClient"; }

  LY_NONCOPYABLE(TcpClient);
protected:
  virtual TcpConnection::ptr onConnect(sockfd_t fd);

protected:
  EventLoop *event_loop_;

  std::unique_ptr<Connector> connector_;
  TcpConnection::ptr conn_;

  std::atomic_bool running_{false};
  Mutex::type mutex_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
