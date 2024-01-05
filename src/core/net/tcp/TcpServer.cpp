#include <core/net/tcp/TcpServer.h>

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(net)
static auto g_net_logger = GET_LOGGER("net");

TcpServer::TcpServer(EventLoop *eventLoop) : event_loop_(eventLoop) {
  acceptor_ =
    std::make_unique<Acceptor>(eventLoop);
  acceptor_->setNewConnectionCallback([this](sockfd_t fd) {
    TcpConnection::ptr conn = this->onConnect(fd);
    if (conn)
    {
      ILOG_INFO(g_net_logger) << "A new comer: " << fd;
      this->addConnection(fd, conn);
      conn->setDisconnectCallback([this](TcpConnection::ptr conn) {
        ILOG_INFO_FMT(g_net_logger, "{} is Leaving...", this->type());
        auto scheduler = conn->getEventLoop();
        sockfd_t sockfd = conn->getSockfd();
        scheduler->addTriggerEventForce(
          [this, sockfd] { this->removeConnection(sockfd); },
          100ms);  // TODO: Replace with env.removeConnectionMsTimeout
      });

      conn->setCloseCallback([](TcpConnection::ptr close_conn) {
        auto &&address = socket_api::getSocketAddr(close_conn->getSockfd());
        ILOG_INFO_FMT(g_net_logger, "A client ({}) leaves", address);
      });

      // TODO: Remove it
      conn->setReadCallback(
        [this](TcpConnection::ptr read_conn, BufferReader &reader) {
          std::string buf{};
          const int readBytes = reader.readAll(buf);
          if (readBytes <= 0 || buf.empty()) return false;

          buf = "Server Echo: you sent " + buf + "recently.\n";
          buf += "Thanks for your response.";
          ILOG_INFO(g_net_logger) << buf;
          read_conn->send(buf.c_str(), buf.length());

          return true;
        });
    }
  });
}

TcpServer::~TcpServer() {
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

  while (!connections_.empty())
  {
    Timer::sleep(10ms);
  }
  return true;
}

TcpConnection::ptr TcpServer::onConnect(sockfd_t fd) {
  return std::make_shared<TcpConnection>(
    event_loop_, fd);
}

void TcpServer::addConnection(sockfd_t fd, TcpConnection::ptr conn) {
  Mutex::lock locker(mutex_);
  connections_.emplace(fd, conn);
}

void TcpServer::removeConnection(sockfd_t fd) {
  Mutex::lock locker(mutex_);
  connections_.erase(fd);
}

NAMESPACE_END(net)

LY_NAMESPACE_END
