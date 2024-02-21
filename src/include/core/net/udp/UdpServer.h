#pragma once
#include <core/net/Acceptor.h>
#include <core/net/EventLoop.h>
#include <core/net/udp/UdpConnection.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
// TODO:
class UdpServer
{
public:
  UdpServer(EventLoop *eventLoop);
  virtual ~UdpServer();

  virtual bool start(const char *ip, uint16_t port, int max_backlog);
  virtual bool stop();

  virtual auto type() const -> std::string { return "UdpServer"; }

  LY_NONCOPYABLE(UdpServer);
protected:
  virtual UdpConnection::ptr onConnect(sockfd_t fd);
  virtual void addConnection(sockfd_t fd, UdpConnection::ptr conn);
  virtual void removeConnection(sockfd_t fd);

  // UdpServerEnv &env_;
  EventLoop *event_loop_;
  std::unique_ptr<Acceptor> acceptor_;

  std::unordered_map<sockfd_t, UdpConnection::ptr> connections_;

  std::atomic_bool running_{false};
  Mutex::type mutex_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
