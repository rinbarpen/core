#include <core/net/Acceptor.h>
#include <core/net/tcp/TcpSocket.h>
#include <core/net/udp/UdpSocket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_net_logger = GET_LOGGER("net");

Acceptor::Acceptor(EventLoop *event_loop, bool is_tcp)
  : event_loop_(event_loop), is_tcp_(is_tcp) {
  accept_callback_ = [this]() {
    this->onAccept();
  };
}
void Acceptor::listen(const char *ip, uint16_t port, int backlog) {
  ILOG_INFO_FMT(g_net_logger, "Acceptor is listening({},{}) on {}", backlog, ip, port);
  Mutex::lock locker(mutex_);
  if (is_tcp_) {
    socket_.reset(new TcpSocket{});
  }
  else {
    socket_.reset(new UdpSocket{});
  }

  socket_api::set_reuse_address(socket_->getSockfd());
  socket_api::set_reuse_port(socket_->getSockfd());
  socket_api::set_nonblocking(socket_->getSockfd());

  if (socket_->bind(ip, port) < 0) {
    throw std::runtime_error("bind");
  }
  channel_.reset(new FdChannel(socket_->getSockfd()));
  if (socket_->listen(backlog) < 0) {
    throw std::runtime_error("listen");
  }

  channel_->setReadCallback([this]() { this->onAccept(); });
  channel_->enableReading();
  event_loop_->updateChannel(channel_);
}
void Acceptor::listen(const NetAddress &addr, int backlog) {
  this->listen(addr.ip.c_str(), addr.port, backlog);
}

void Acceptor::close() {
  ILOG_INFO(g_net_logger) << "Acceptor is closed";

  Mutex::lock locker(mutex_);
  if (socket_->isValid()) {
    event_loop_->removeChannel(channel_->getSockfd());
    socket_->close();
  }
}
void Acceptor::setNewConnectionCallback(NewConnectionCallback callback) {
  new_connection_callback_ = callback;
}

void Acceptor::onAccept() {
  Mutex::lock locker(mutex_);
  auto &&[target, _] = socket_->accept();
  if (socket_api::is_valid(target)) {
    ILOG_INFO(g_net_logger) << "Acceptor accepts a new client: " << target;
    if (new_connection_callback_)
      new_connection_callback_(target);
    else
      socket_api::close(target);
  }
}

NAMESPACE_END(net)
LY_NAMESPACE_END
