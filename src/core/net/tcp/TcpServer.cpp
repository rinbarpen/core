#include <chrono>
#include <core/config/config.h>
#include <core/net/tcp/TcpServer.h>

NAMESPACE_BEGIN(ly::net)
static auto g_net_logger = GET_LOGGER("net.tcp");
static auto g_timeout_while_removing_connections_ms = LY_CONFIG_GET(net.common.timeout_while_removing_connections_ms);

TcpServer::TcpServer(EventLoop *event_loop) : event_loop_(event_loop) {
  acceptor_ =
    std::make_unique<Acceptor>(event_loop);
  acceptor_->setNewConnectionCallback([this](sockfd_t fd) {
    TcpConnection::ptr conn = this->onConnect(fd);
    if (conn) {
      ILOG_INFO_FMT(g_net_logger, "A new comer: {}", fd);
      this->addConnection(fd, conn);
      conn->setDisconnectCallback([this](TcpConnection::ptr conn) {
        ILOG_INFO_FMT(g_net_logger, "{} is Leaving...", this->type());
        auto scheduler = conn->getTaskScheduler();
        sockfd_t sockfd = conn->getSockfd();
        scheduler->addTriggerEventForce(
          [this, sockfd] { this->removeConnection(sockfd); },
          std::chrono::milliseconds{g_timeout_while_removing_connections_ms});
      });

      conn->setCloseCallback([](TcpConnection::ptr close_conn) {
        auto &&address = socket_api::getSocketAddr(close_conn->getSockfd());
        ILOG_INFO_FMT(g_net_logger, "A client ({}) leaves", address);
      });

      // TODO: Remove it
      // conn->setReadCallback(
      //   [this](TcpConnection::ptr read_conn, BufferReader &reader) {
      //     std::string buf{};
      //     const int readBytes = reader.readAll(buf);
      //     if (readBytes <= 0 || buf.empty()) return false;

      //     buf = "Server Echo: you sent " + buf + "recently.\n";
      //     buf += "Thanks for your response.";
      //     ILOG_INFO(g_net_logger) << buf;
      //     read_conn->send(buf.c_str(), buf.length());

      //     return true;
      //   });
    }
  });
}

TcpServer::~TcpServer() {
  if (running_)
    (void)this->stop();
}

bool TcpServer::start(const char *ip, uint16_t port, int max_backlog) {
  if (running_) return false;

  ILOG_INFO_FMT(g_net_logger, "{} is started", this->type());
  acceptor_->listen(ip, port, max_backlog);
  running_ = true;
  return true;
}

bool TcpServer::stop() {
  if (!running_) return false;

  running_ = false;
  ILOG_INFO_FMT(g_net_logger, "{} is stopping", this->type());
  ILOG_INFO_FMT(g_net_logger, "There are {} connections", connections_.size());
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

TcpConnection::ptr TcpServer::onConnect(sockfd_t fd) {
  return std::make_shared<TcpConnection>(
    event_loop_->getTaskScheduler().get(), fd);
}

void TcpServer::addConnection(sockfd_t fd, TcpConnection::ptr conn) {
  Mutex::lock locker(mutex_);
  connections_.emplace(fd, conn);
}

void TcpServer::removeConnection(sockfd_t fd) {
  Mutex::lock locker(mutex_);
  connections_.erase(fd);
}

NAMESPACE_END(ly::net)
