#include <core/plugin/Download.h>

LY_NAMESPACE_BEGIN
Downloader::Downloader(net::EventLoop::ptr pEventLoop)
  : event_loop_(pEventLoop)
{}
void Downloader::download(std::string url, size_t nbytes, std::string saved_path, bool is_deamon)
{
  // http instead
  tcp_client_.reset(new net::TcpClient{event_loop_.get()});
  tcp_client_->start(url.c_str(), 8080);

  thread_pool_->start();
  size_t nbytes_per_thread = (nbytes + thread_pool_->capacity() - 1) / thread_pool_->capacity();
  size_t curr = 0;
  for (size_t i = 0; i < thread_pool_->capacity(); ++i) {
    // (void)thread_pool_->submit([=]{work(url, curr, nbytes_per_thread);});
    if (curr + nbytes_per_thread > nbytes) {
      (void)thread_pool_->submit(&Downloader::work, this, curr, nbytes - curr);
    }
    else {
      (void)thread_pool_->submit(&Downloader::work, this, curr, nbytes_per_thread);
    }
    curr += nbytes_per_thread;
  }
  if (!is_deamon) {
    thread_pool_->stop();
  }
}

void Downloader::work(size_t start_block, size_t blocks)
{
  Mutex::lock locker(mutex_);
}
LY_NAMESPACE_END
