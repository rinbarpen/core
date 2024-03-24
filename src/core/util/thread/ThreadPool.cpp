#include <core/util/thread/ThreadPool.h>

#include <fmt/core.h>

LY_NAMESPACE_BEGIN

ThreadPool::ThreadPool(size_t nThread, bool autoRun) noexcept
  : capacity_(nThread)
{
  threads_.reserve(nThread);
  for (size_t i = 0; i < nThread; ++i) {
    threads_.emplace_back(Thread(fmt::format("ThreadPool[{}]", i)));
  }

  if (autoRun) {
    for (auto &th : threads_) {
      th.dispatch(&ThreadPool::work, this);
    }
    running_ = true;
  }
}
ThreadPool::~ThreadPool() noexcept
{
  if (!running_) return;

  running_ = false;
  cond_.notify_all();
  for (auto &th : threads_) {
    th.stop();
  }
}

void ThreadPool::start() noexcept
{
  if (running_) {
    return;
  }

  for (auto &th : threads_) {
    th.dispatch(&ThreadPool::work, this);
  }

  running_ = true;
}
void ThreadPool::stop() noexcept
{
  if (!running_) {
    return;
  }

  running_ = false;
  cond_.notify_all();
  for (auto &th : threads_) {
    th.dispatch(&ThreadPool::work, this);
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
