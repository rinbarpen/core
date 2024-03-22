#pragma once

#include <unordered_map>

#include <core/util/Mutex.h>
#include <core/net/Acceptor.h>
#include <core/net/tcp/TcpConnection.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class TcpServer
{
public:
  explicit TcpServer(EventLoop *event_loop);
  virtual ~TcpServer();

  bool start(const char *ip, uint16_t port, int max_backlog);
  bool stop();

  virtual std::string type() const { return "TcpServer"; }

  LY_NONCOPYABLE(TcpServer);
protected:
  virtual TcpConnection::ptr onConnect(sockfd_t fd);
  virtual void addConnection(sockfd_t fd, TcpConnection::ptr conn);
  virtual void removeConnection(sockfd_t fd);

  EventLoop *event_loop_;
  std::unique_ptr<Acceptor> acceptor_;

  std::unordered_map<sockfd_t, TcpConnection::ptr> connections_;

  std::atomic_bool running_{false};
	mutable Mutex::type mutex_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
