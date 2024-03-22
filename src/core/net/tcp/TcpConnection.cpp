#include <core/util/marcos.h>
#include <core/config/config.h>
#include <core/net/TaskScheduler.h>
#include <core/net/tcp/TcpConnection.h>
#include <cstring>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_send_timeout_ms = LY_CONFIG_GET(net.common.send_timeout_ms);
static auto g_max_sender_buffer_size = LY_CONFIG_GET(net.common.max_sender_buffer_size);
static auto g_max_receiver_buffer_size = LY_CONFIG_GET(net.common.max_receiver_buffer_size);

TcpConnection::TcpConnection(TaskScheduler *task_scheduler, sockfd_t fd)
  : task_scheduler_(task_scheduler),
  read_buffer_(new BufferReader(g_max_receiver_buffer_size)),
  write_buffer_(new BufferWriter(g_max_sender_buffer_size)),
  channel_(new FdChannel(fd))
{
  channel_->setReadCallback([this]() { this->onRead(); });
  channel_->setWriteCallback([this]() { this->onWrite(); });
  channel_->setCloseCallback([this]() { this->onClose(); });
  channel_->setErrorCallback([this](){ this->onError(); });

  socket_api::set_nonblocking(fd);
  socket_api::set_send_buffer_size(fd, 100 * 1024);
  socket_api::set_keep_alive(fd);

  channel_->enableReading();
  task_scheduler_->updateChannel(channel_);
}

TcpConnection::~TcpConnection()
{
  this->close();
}

void TcpConnection::send(const char* data, size_t len)
{
  if (running_) {
    {
      Mutex::lock locker(mutex_);
      write_buffer_->append(data, len);
    }

    this->onWrite();
  }
}
size_t TcpConnection::recv(char *data, size_t len)
{
  if (running_) {
    size_t r{};
    {
      Mutex::lock locker(mutex_);
      r = read_buffer_->read(data, (uint32_t)len);
    }

    this->onRead();
    return r;
  }
  return 0;
}
void TcpConnection::recv(std::string &data, size_t len)
{
  if (running_) {
    {
      Mutex::lock locker(mutex_);
      data = read_buffer_->read((uint32_t)len);
    }

    this->onRead();
  }
}

void TcpConnection::disconnect()
{
  auto conn = shared_from_this();
  Mutex::lock locker(mutex_);
  task_scheduler_->addTriggerEventForce([conn]() {
    conn->close();
  }, 0ms);
}

void TcpConnection::onRead()
{
  if (!running_) return;
  Mutex::lock locker(mutex_);

  int r = read_buffer_->readFromSocket(channel_->getSockfd());
  if (r <= 0) {
    this->close();
    return;
  }

  if (read_callback_) {
    if (!read_callback_(shared_from_this(), *read_buffer_)) {
      this->close();
      return;
    }
  }
}
void TcpConnection::onWrite()
{
  if (!running_) return;
  Mutex::lock locker(mutex_);

  int r = write_buffer_->send(channel_->getSockfd(), g_send_timeout_ms);
  if (r < 0) {
    this->close();
    return;
  }

  if (write_buffer_->empty()) {
    if (channel_->isWriting()) {
      channel_->disableWriting();
      task_scheduler_->updateChannel(channel_);
    }
  }
  else if (!channel_->isWriting()) {
    channel_->enableWriting();
    task_scheduler_->updateChannel(channel_);
  }
}
void TcpConnection::onClose()
{
  Mutex::lock locker(mutex_);
  this->close();
}
void TcpConnection::onError()
{
  Mutex::lock locker(mutex_);
  this->close();
}
void TcpConnection::onDisconnect()
{
  Mutex::lock locker(mutex_);
  this->close();
}

void TcpConnection::close()
{
  if (running_) {
    running_ = false;
    task_scheduler_->removeChannel(channel_->getSockfd());

    if (close_callback_) {
      close_callback_(shared_from_this());
    }

    if (disconnect_callback_) {
      disconnect_callback_(shared_from_this());
    }
  }
}
NAMESPACE_END(net)
LY_NAMESPACE_END
