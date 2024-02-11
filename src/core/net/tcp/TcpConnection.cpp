#include "core/config/config.h"
#include "core/net/TaskScheduler.h"
#include <core/net/tcp/TcpConnection.h>

NAMESPACE_BEGIN(ly::net)
static auto g_send_timeout_ms = LY_CONFIG_GET(net.common.send_timeout_ms);

TcpConnection::TcpConnection(TaskScheduler *task_scheduler, sockfd_t fd)
  : task_scheduler_(task_scheduler),
  read_buffer_(new BufferReader(kMaxBufferSize)),
  write_buffer_(new BufferWriter(kMaxBufferSize)),
  channel_(new FdChannel(fd))
{
  channel_->setReadCallback([this]() { this->onRead(); });
  channel_->setWriteCallback([this]() { this->onWrite(); });
  channel_->setCloseCallback([this]() { this->onClose(); });
  channel_->setErrorCallback([this](){ this->onError(); });

  socket_api::setNonBlocking(fd);
  socket_api::setSendBufSize(fd, 100 * 1024);
  socket_api::setKeepAlive(fd);

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
  Mutex::lock locker(mutex_);
  if (!running_) return;

  int r = read_buffer_->read(channel_->getSockfd());
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
  Mutex::lock locker(mutex_);
  if (!running_) return;

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
NAMESPACE_END(ly::net)
