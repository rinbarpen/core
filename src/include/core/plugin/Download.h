#pragma once

#include <memory>
#include <string_view>

#include <core/util/marcos.h>
#include <core/util/thread/ThreadPool.h>
#include <core/net/tcp/TcpConnection.h>

LY_NAMESPACE_BEGIN
class Downloader
{
public:
  Downloader() = default;
  ~Downloader() = default;

  void download(std::string_view url, std::string_view saved_path);

private:
  void work(std::string_view url, size_t start_block, size_t blocks);

private:
  std::shared_ptr<ThreadPool> thread_pool_;
  char *buffer_{nullptr};
  size_t max_buffer_size_;
  size_t downloaded_nbytes_{0};
  size_t total_nbytes_{0};
  net::TcpConnection::ptr tcp_connection_;
};
LY_NAMESPACE_END
