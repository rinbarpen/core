#include <core/net/Connector.h>
#include <core/net/tcp/TcpSocket.h>
#include <core/net/udp/UdpSocket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

Connector::Connector(EventLoop* EventLoop) :
  event_loop_(EventLoop), next_retry_interval_(500), retry_times_(5),
  socket_(new TcpSocket())
{
}

Connector::~Connector()
{

}

bool Connector::connect(const char* ip, uint16_t port)
{
  if (connecting_ || retry_times_ <= 0) return false;

  sockfd_t server_fd = socket_api::connect(socket_->getSockfd(), ip, port);
  if (server_fd == kInvalidSockfd) {
    event_loop_->addTriggerEventForce([&]() {
      next_retry_interval_ *= 2;
      if (next_retry_interval_ > 30000) {
        next_retry_interval_ = 30000;
      }
      if (!this->connect(ip, port)) {
        retry_times_--;
      }
    }, std::chrono::milliseconds(next_retry_interval_));
  } else {
    connect_cb_(server_fd);
    next_retry_interval_ = 500;
    retry_times_ = 5;
    connecting_ = true;

    return true;
  }

  return false;
}

void Connector::close()
{
  if (connecting_) {
    if (socket_ && socket_->isValid()) {
      socket_->close();
    }

    if (disconnect_cb_) {
      disconnect_cb_();
    }

    connecting_ = false;
  }
}

NAMESPACE_END(net)
LY_NAMESPACE_END
