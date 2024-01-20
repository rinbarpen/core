#pragma once

#include <core/net/tcp/TcpConnection.h>
#include <core/net/Connector.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class TcpClient
{
public:
  // TcpClient(EventLoop *eventLoop, TcpClientEnv &env);
  TcpClient(EventLoop *eventLoop);
  virtual ~TcpClient();

  virtual bool start(const char *ip, uint16_t port);
  virtual bool stop();

  virtual auto type() const -> std::string { return "TcpClient"; }

  LY_NONCOPYABLE(TcpClient);
protected:
  virtual TcpConnection::ptr onConnect(sockfd_t fd);

protected:
  // TcpClientEnv &env_;
  EventLoop *event_loop_;

  std::unique_ptr<Connector> connector_;
  TcpConnection::ptr conn_;

  std::atomic_bool running_{false};
  Mutex::type mutex_;
};
NAMESPACE_END(net)

LY_NAMESPACE_END
