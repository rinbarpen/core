#include <core/net/tcp/TcpClient.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_net_tcp_logger = GET_LOGGER("net.tcp");

TcpClient::TcpClient(EventLoop *event_loop)
  : event_loop_(event_loop)
  , connector_(new Connector(event_loop)) {
  connector_->setConnectCallback(
    [&](sockfd_t fd) { conn_ = this->onConnect(fd); });
  connector_->setDisconnectCallback([this]() {
    ILOG_INFO_FMT(g_net_tcp_logger, "{} is Leaving ...", this->type());
  });
}
TcpClient::~TcpClient()
{
  (void)this->stop();
}

bool TcpClient::start(const char *ip, uint16_t port) {
  if (running_) return false;

  ILOG_INFO_FMT(g_net_tcp_logger, "Start {} ...", this->type());
  if (!connector_->connect(ip, port)) { return false; }
  running_ = true;

  return true;
}
bool TcpClient::stop() {
  if (!running_) return false;

  ILOG_INFO_FMT(g_net_tcp_logger, "{} is STOPPING", this->type());

  running_ = false;
  conn_->disconnect();
  connector_->close();

  return true;
}

TcpConnection::ptr TcpClient::onConnect(sockfd_t fd) {
  return std::make_shared<TcpConnection>(
    event_loop_->getTaskScheduler().get(), fd);
}

NAMESPACE_END(net)
LY_NAMESPACE_END
