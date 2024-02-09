#pragma once

#include <thread>
#include <future>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <condition_variable>
#include <functional>

#include <core/util/marcos.h>
#include <core/util/ds/SafeQueue.h>
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
      if (!running_) {
        return {};
      }

      std::lock_guard<std::mutex> locker(queue_mutex_);
      tasks_.push([task, &args...] {
        (*task)(std::forward<Args>(args)...);
      });
    }

    cond_.notify_one();
    return res;
  }

  void start() noexcept;
  void stop() noexcept;

  size_t capacity() const noexcept { return capacity_; }

private:
  void work();

private:
  std::vector<std::thread> threads_;
  std::size_t capacity_;

  std::atomic_bool running_{false};

  // queue
  std::condition_variable cond_;
  std::mutex queue_mutex_;
  std::queue<TaskCallback> tasks_;
};

LY_NAMESPACE_END
