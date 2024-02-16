#pragma once
#include <cstdint>

#include <core/util/buffer/BufferReader.h>
#include <core/util/buffer/BufferWriter.h>
#include <core/net/NetAddress.h>
#include <core/net/SocketUtil.h>
#include <core/net/TaskScheduler.h>
#include <core/net/FdChannel.h>
#include <core/net/EventLoop.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
protected:
  static constexpr int kMaxBufferSize = 1024 * 1024;
public:
  SHARED_REG(TcpConnection);

  using ReadCallback = std::function<bool(TcpConnection::ptr, BufferReader&)>;
  using DisconnectCallback = std::function<void(TcpConnection::ptr)>;
  using CloseCallback = std::function<void(TcpConnection::ptr)>;
  using ErrorCallback = std::function<void(TcpConnection::ptr)>;

  TcpConnection(TaskScheduler* task_scheduler, sockfd_t fd);
  virtual ~TcpConnection();

  void send(const char *data, size_t len);
  void recv(std::string &data, size_t len);

  void disconnect();

  bool isRunning() const { return running_; }

  void setReadCallback(ReadCallback callback) { read_callback_ = callback; }
  void setDisconnectCallback(DisconnectCallback callback) { disconnect_callback_ = callback; }
  void setCloseCallback(CloseCallback callback) { close_callback_ = callback; }
  void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }

  TaskScheduler* getTaskScheduler() const { return task_scheduler_; }
  sockfd_t getSockfd() const { return channel_->getSockfd(); }
  NetAddress getAddress() const { return socket_api::peer_address(channel_->getSockfd()); }

  uintptr_t getId() const {
    return reinterpret_cast<uintptr_t>(task_scheduler_);
  }

protected:
  virtual void onRead();
  virtual void onWrite();
  virtual void onClose();
  virtual void onError();
  virtual void onDisconnect();

  std::atomic_bool running_{false};
  std::unique_ptr<BufferReader> read_buffer_;
  std::unique_ptr<BufferWriter> write_buffer_;
  TaskScheduler *task_scheduler_;

private:
  void close();

private:
  ReadCallback read_callback_;
  DisconnectCallback disconnect_callback_;
  CloseCallback close_callback_;
  ErrorCallback error_callback_;

  FdChannel::ptr channel_;

  Mutex::type mutex_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
