#pragma once

#include <unordered_map>
#include <core/net/tcp/TcpConnection.h>
#include <core/net/Acceptor.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class TcpServer
{
public:
  TcpServer(EventLoop *eventLoop);
  virtual ~TcpServer();

  virtual bool start(const char *ip, uint16_t port, int max_backlog);
  virtual bool stop();

  void doEventLoop() const { event_loop_->start(); }

  virtual auto type() const -> std::string { return "TcpServer"; }

  LY_NONCOPYABLE(TcpServer);
protected:
  virtual TcpConnection::ptr onConnect(sockfd_t fd);
  virtual void addConnection(sockfd_t fd, TcpConnection::ptr conn);
  virtual void removeConnection(sockfd_t fd);

  // TcpServerEnv &env_;
  EventLoop *event_loop_;
  std::unique_ptr<Acceptor> acceptor_;

  std::unordered_map<sockfd_t, TcpConnection::ptr> connections_;

  std::atomic_bool running_{false};
  Mutex::type mutex_;
};

NAMESPACE_END(net)

LY_NAMESPACE_END
