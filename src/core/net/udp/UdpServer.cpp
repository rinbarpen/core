#include <core/config/config.h>
#include <core/net/udp/UdpServer.h>
#include <core/util/logger/Logger.h>
#include "core/net/udp/UdpConnection.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_net_udp_logger = GET_LOGGER("net.udp");
static auto g_timeout_while_removing_connections_ms = LY_CONFIG_GET(net.common.timeout_while_removing_connections_ms);

UdpServer::UdpServer(EventLoop *event_loop)
  : event_loop_(event_loop) {
  acceptor_ =
    std::make_unique<Acceptor>(event_loop);
  acceptor_->setNewConnectionCallback([this](sockfd_t fd) {
    UdpConnection::ptr conn = this->onConnect(fd);
    if (conn) {
      ILOG_INFO_FMT(g_net_udp_logger, "A new comer: {}", fd);
      this->addConnection(fd, conn);
      conn->setDisconnectCallback([this](UdpConnection::ptr conn) {
        ILOG_INFO_FMT(g_net_udp_logger, "{} is Leaving...", this->type());
        auto scheduler = conn->getTaskScheduler();
        sockfd_t sockfd = conn->getSockfd();
        scheduler->addTriggerEventForce(
          [this, sockfd] { this->removeConnection(sockfd); },
          std::chrono::milliseconds{g_timeout_while_removing_connections_ms});
      });

      conn->setCloseCallback([](UdpConnection::ptr close_conn) {
        auto &&address = socket_api::socket_address(close_conn->getSockfd());
        ILOG_INFO_FMT(g_net_udp_logger, "A client ({}) leaves", address);
      });
    }
  });
}
UdpServer::~UdpServer() {
  (void)this->stop();
}

bool UdpServer::start(const char *ip, uint16_t port, int max_backlog) {
  if (running_) return false;

  ILOG_INFO_FMT(g_net_udp_logger, "{} is started", this->type());
  acceptor_->listen(ip, port, max_backlog);
  running_ = true;
  return true;
}
bool UdpServer::stop() {
  if (!running_) return false;

  running_ = false;
  ILOG_INFO_FMT(g_net_udp_logger, "{} is stopping", this->type());
  ILOG_INFO_FMT(g_net_udp_logger, "There are {} connections", connections_.size());
  {
    Mutex::lock locker(mutex_);
    for (auto &[fd, conn] : connections_)
    { conn->disconnect(); }
  }

  acceptor_->close();
  while (!connections_.empty()) {
    Timer::sleep(10ms);
  }
  return true;
}

UdpConnection::ptr UdpServer::onConnect(sockfd_t fd) {
  return std::make_shared<UdpConnection>(
    event_loop_->getTaskScheduler().get(), fd);
}

void UdpServer::addConnection(sockfd_t fd, UdpConnection::ptr conn) {
  Mutex::lock locker(mutex_);
  connections_.emplace(fd, conn);
}

void UdpServer::removeConnection(sockfd_t fd) {
  Mutex::lock locker(mutex_);
  connections_.erase(fd);
}

NAMESPACE_END(net)
LY_NAMESPACE_END
