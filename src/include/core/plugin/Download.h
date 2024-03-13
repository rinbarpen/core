#pragma once

#include <memory>

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/util/thread/ThreadPool.h>
#include <core/net/tcp/TcpConnection.h>
#include "core/net/EventLoop.h"
#include "core/net/tcp/TcpClient.h"

LY_NAMESPACE_BEGIN
class Downloader
{
public:
  Downloader(net::EventLoop::ptr pEventLoop);
  ~Downloader() = default;

  void download(std::string url, size_t nbytes, std::string saved_path, bool is_deamon = false);

private:
  void work(size_t start_block, size_t blocks);

private:
  std::unique_ptr<ThreadPool> thread_pool_;
  char *buffer_{nullptr};
  size_t max_buffer_size_;
  size_t downloaded_nbytes_{0};
  size_t total_nbytes_{0};
  size_t max_cache_size_{4*1024};
  Mutex::type mutex_;
  net::TcpClient::ptr tcp_client_;
  net::EventLoop::ptr event_loop_;
};
LY_NAMESPACE_END
