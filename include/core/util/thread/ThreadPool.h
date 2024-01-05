#pragma once

#include <thread>
#include <future>
#include <type_traits>
#include <utility>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <functional>

#include <core/util/marcos.h>
// #include <core/util/thread/Thread.h>

LY_NAMESPACE_BEGIN

class ThreadPool
{
public:
  using TaskCallback = std::function<void()>;

  ThreadPool(size_t nThread = std::thread::hardware_concurrency(), bool autoRun = true) noexcept;
  ~ThreadPool() noexcept;

  template <typename Fn, typename... Args>
  LY_NODISCARD auto submit(Fn &&fn, Args &&...args)
    -> std::future<std::invoke_result_t<Fn, Args...>>
  {
    using ReturnType = std::invoke_result_t<Fn, Args...>;

    auto task = std::make_shared<std::packaged_task<ReturnType(Args...)>>(std::forward<Fn>(fn));

    auto res = task->get_future();
    {
      std::lock_guard<std::mutex> locker(queue_mutex_);
      if (!running_) {
        throw ThreadPoolException("Submit task when ThreadPool is stopping");
      }

      tasks_.push([task, &args...] {
        (*task)(std::forward(args)...);
      });
    }

    cond_.notify_one();
    return res;
  }

  void start();
  void stop();

  size_t capacity() const { return capacity_; }

private:
  void work();

private:
  class ThreadPoolException : std::runtime_error
  {
  public:
    ThreadPoolException(const char *msg)
      : std::runtime_error(msg)
    {}

  };

private:
  std::vector<std::thread> threads_;
  std::size_t capacity_;

  std::atomic_bool running_{false};

  // queue
  std::mutex queue_mutex_;
  std::condition_variable cond_;
  std::queue<TaskCallback> tasks_;
};

LY_NAMESPACE_END
