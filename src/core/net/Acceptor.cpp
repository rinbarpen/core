#include <core/net/Acceptor.h>

#include <core/net/tcp/TcpSocket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_net_logger = GET_LOGGER("net");

Acceptor::Acceptor(EventLoop* pEventLoop)
  : event_loop_(pEventLoop)
{
  accept_cb_ = [this]() {
    this->onAccept();
  };
}
void Acceptor::listen(const char* ip, uint16_t port, int backlog)
{
  this->listen({ip, port}, backlog);
}
void Acceptor::listen(const NetAddress& addr, int backlog)
{
  ILOG_INFO_FMT(g_net_logger,
   "Acceptor is listening({}) on {}",
   backlog, addr);
  Mutex::lock locker(mutex_);
  socket_.reset(new TcpSocket());

  socket_api::setReuseAddr(socket_->getSockfd());
  socket_api::setReusePort(socket_->getSockfd());
  socket_api::setNonBlocking(socket_->getSockfd());

  if (socket_->bind(addr) < 0) {
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

void Acceptor::close()
{
  ILOG_INFO(g_net_logger) << "Acceptor is closed";

  Mutex::lock locker(mutex_);
  if (socket_->isValid()) {
    event_loop_->removeChannel(channel_->getSockfd());
    socket_->close();
  }
}

void Acceptor::onAccept()
{
  Mutex::lock locker(mutex_);
  auto &&[target, _] = socket_->accept();
  if (target != kInvalidSockfd) {
    ILOG_INFO(g_net_logger) << "Acceptor accepts a new client: " << target;
    if (new_connection_cb_) new_connection_cb_(target);
    else socket_api::close(target);
  }
}

NAMESPACE_END(net)

LY_NAMESPACE_END
