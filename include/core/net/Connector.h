#pragma once
#include <functional>

#include <core/net/EventLoop.h>
#include <core/net/SocketUtil.h>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(net)

class Connector
{
public:
  using ConnectCallback = std::function<void(sockfd_t)>;
  using DisconnectCallback = std::function<void()>;

  Connector(EventLoop* eventLoop);
  ~Connector();

  bool connect(const char *ip, uint16_t port);
  void close();

  void setConnectCallback(ConnectCallback callback) { connect_cb_ = callback; }
  void setDisconnectCallback(DisconnectCallback callback) {
    disconnect_cb_ = callback;
  }

  bool isConnecting() const { return connecting_; }

  LY_NONCOPYABLE(Connector);
private:
  EventLoop* event_loop_;

  std::unique_ptr<Socket> socket_;
  NetAddress addr_;
  int next_retry_interval_;
  ConnectCallback connect_cb_;
  DisconnectCallback disconnect_cb_;
  int retry_times_;

  bool connecting_{false};
};

NAMESPACE_END(net)

LY_NAMESPACE_END
