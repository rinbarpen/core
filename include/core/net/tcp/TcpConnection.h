#pragma once
#include <core/net/FdChannel.h>
#include <core/net/EventLoop.h>
#include <core/util/buffer/Buffer.h>
#include <core/util/buffer/BufferReader.h>
#include <core/util/buffer/BufferWriter.h>

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

  TcpConnection(EventLoop* pEventLoop, sockfd_t fd);
  virtual ~TcpConnection();

  void send(const char *data, size_t len);
  void disconnect();

  bool isRunning() const { return running_; }

  void setReadCallback(ReadCallback fn) { read_cb_ = fn; }
  void setDisconnectCallback(DisconnectCallback fn) { disconnect_cb_ = fn; }
  void setCloseCallback(CloseCallback fn) { close_cb_ = fn; }
  void setErrorCallback(ErrorCallback fn) { error_cb_ = fn; }

  sockfd_t getSockfd() const { return channel_->getSockfd(); }
  EventLoop* getEventLoop() const { return event_loop_; }

protected:
  virtual void onRead();
  virtual void onWrite();
  virtual void onClose();
  virtual void onError();
  virtual void onDisconnect();

  std::atomic_bool running_{false};
  std::unique_ptr<BufferReader> read_buffer_;
  std::unique_ptr<BufferWriter> write_buffer_;
  EventLoop *event_loop_;

private:
  void close();

private:
  ReadCallback read_cb_;
  DisconnectCallback disconnect_cb_;
  CloseCallback close_cb_;
  ErrorCallback error_cb_;

  FdChannel::ptr channel_;

  Mutex::type mutex_;
};

NAMESPACE_END(net)

LY_NAMESPACE_END
