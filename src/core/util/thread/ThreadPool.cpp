#include <core/util/thread/ThreadPool.h>

LY_NAMESPACE_BEGIN

ThreadPool::ThreadPool(size_t nThread, bool autoRun)
  : capacity_(nThread)
{
  threads_.reserve(nThread);
  for (size_t i = 0; i < nThread; ++i)
    threads_.emplace_back(std::thread(&ThreadPool::work, this));

  if (autoRun) running_ = true;
}
ThreadPool::~ThreadPool() 
{
  running_ = false;
  
  for (auto &th : threads_) 
  {
    th.join();
  }
}

template<typename Fn, typename... Args>
auto ThreadPool::submit(Fn &&fn, Args &&...args) -> std::future<std::result_of_t<Fn(Args...)>>
{
  using ReturnType = std::result_of_t<Fn(Args...)>;

  std::packaged_task<ReturnType(Args...)> task(
    std::forward<Fn>(fn), std::forward<Args>(args)...
  );

  auto res = task.get_future();
  {
    std::lock_guard<std::mutex> locker(queue_mutex_);
    if (!running_) {
      throw ThreadPoolException("Submit task when ThreadPool is stopping");
    }

    tasks_.emplace([task]() {task();});
  }

  cond_.notify_one();
  return res;
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
  for (auto &th : threads_)
  {
    th.join();
  }
}

void ThreadPool::work()
{
  TaskCallback task;
  for (;;) {
    {
      std::unique_lock<std::mutex> locker(queue_mutex_);
      cond_.wait(locker, [this]() {
        return !running_ && !tasks_.empty();
      });
      if (!running_ && tasks_.empty())
        return;

      task = std::move(tasks_.front()); tasks_.pop();
    }
    task();
  }
}

LY_NAMESPACE_END
