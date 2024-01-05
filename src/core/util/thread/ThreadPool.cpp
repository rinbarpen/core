#include <core/util/thread/ThreadPool.h>

LY_NAMESPACE_BEGIN

ThreadPool::ThreadPool(size_t nThread, bool autoRun) noexcept
  : capacity_(nThread)
{
  threads_.reserve(nThread);
  for (size_t i = 0; i < nThread; ++i)
    threads_.emplace_back(&ThreadPool::work, this);

  if (autoRun) {
    running_ = true;
  }
}
ThreadPool::~ThreadPool() noexcept
{
  if (!running_) return;

  running_ = false;
  cond_.notify_all();
  for (auto &th : threads_)
  {
    if (th.joinable()) th.join();
  }
}

void ThreadPool::start()
{
  if (running_) {
    throw ThreadPoolException("ThreadPool has been started");
    return;
  }

  for (auto &th : threads_)
  {
    th = std::thread(&ThreadPool::work, this);
  }

  running_ = true;
}
void ThreadPool::stop()
{
  if (!running_) {
    throw ThreadPoolException("ThreadPool has been stopped");
    return;
  }

  running_ = false;
  cond_.notify_all();
  for (auto &th : threads_)
  {
    if (th.joinable()) th.join();
  }
}

void ThreadPool::work()
{
  TaskCallback task;
  for (;;) {
    {
      std::unique_lock<std::mutex> locker(queue_mutex_);
      cond_.wait(locker, [this]() {
        return !running_ || !tasks_.empty();
      });
      if (!running_ && tasks_.empty())
        return;

      task = std::move(tasks_.front()); tasks_.pop();
    }
    task();
  }
}

LY_NAMESPACE_END
