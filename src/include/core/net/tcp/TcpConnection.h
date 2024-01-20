#pragma once
#include "core/net/NetAddress.h"
#include "core/net/SocketUtil.h"
#include "core/net/TaskScheduler.h"
#include <core/net/FdChannel.h>
#include <core/net/EventLoop.h>
#include <core/util/buffer/BufferReader.h>
#include <core/util/buffer/BufferWriter.h>
#include <cstdint>

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

  TcpConnection(TaskScheduler* pTaskScheduler, sockfd_t fd);
  virtual ~TcpConnection();

  void send(const char *data, size_t len);
  void disconnect();

  bool isRunning() const { return running_; }

  void setReadCallback(ReadCallback fn) { read_callback_ = fn; }
  void setDisconnectCallback(DisconnectCallback fn) { disconnect_callback_ = fn; }
  void setCloseCallback(CloseCallback fn) { close_callback_ = fn; }
  void setErrorCallback(ErrorCallback fn) { error_callback_ = fn; }

  TaskScheduler* getTaskScheduler() const { return task_scheduler_; }
  sockfd_t getSockfd() const { return channel_->getSockfd(); }
  auto getAddress() const -> NetAddress { return socket_api::getPeerAddr(channel_->getSockfd()); }

  auto getId() const -> int { return reinterpret_cast<uintptr_t>(task_scheduler_); }

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
