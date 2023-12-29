#include <core/net/tcp/TcpClient.h>

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(net)
static auto g_net_logger = GET_LOGGER("net");

TcpClient::TcpClient(EventLoop *eventLoop)
  : event_loop_(eventLoop)
  , connector_(new Connector(eventLoop)) {
  connector_->setConnectCallback(
    [&](sockfd_t fd) { conn_ = this->onConnect(fd); });
  connector_->setDisconnectCallback([this]() {
    ILOG_INFO_FMT(g_net_logger, "Leaving ...");
  });
}
TcpClient::~TcpClient()
{
  if (running_) this->stop();
}

bool TcpClient::start(const char *ip, uint16_t port) {
  if (running_) return false;

  ILOG_INFO(g_net_logger) << "Start TcpClient...";
  connector_->connect(ip, port);
  running_ = true;

  return true;
}
bool TcpClient::stop() {
  if (!running_) return false;

  ILOG_DEBUG(g_net_logger) << "TcpClient is STOP";

  running_ = false;
  conn_->disconnect();
  connector_->close();

  return true;
}

TcpConnection::ptr TcpClient::onConnect(sockfd_t fd) {
  return std::make_shared<TcpConnection>(
    event_loop_, fd);
}

NAMESPACE_END(net)

LY_NAMESPACE_END
