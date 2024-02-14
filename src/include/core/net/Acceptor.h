#pragma once
#include <core/net/EventLoop.h>
#include <core/net/FdChannel.h>
#include <core/net/Socket.h>
#include <core/util/marcos.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class Acceptor
{
public:
  SHARED_REG(Acceptor);

  using AcceptCallbackFn = std::function<void()>;
  using NewConnectionCallback = std::function<void(sockfd_t)>;

  Acceptor(EventLoop* event_loop);
  ~Acceptor() = default;

  void listen(const char *ip, uint16_t port, int backlog);
  void listen(const NetAddress &addr, int backlog);
  void close();

  void setNewConnectionCallback(NewConnectionCallback callback);
  // bool isIpv6() const { return socket_->isIpv6(); }

  std::string getIp() const { return socket_->getIp(); }
  uint16_t getPort() const { return socket_->getPort(); }
private:
  void onAccept();

private:
  EventLoop *event_loop_;
  std::unique_ptr<Socket> socket_;
  AcceptCallbackFn accept_callback_;
  NewConnectionCallback new_connection_callback_;
  FdChannel::ptr channel_;

  Mutex::type mutex_;
};

NAMESPACE_END(net)

LY_NAMESPACE_END
