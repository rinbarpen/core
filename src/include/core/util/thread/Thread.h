#pragma once

#include <atomic>
#include <thread>
#include <future>

#include <core/util/marcos.h>
#include <core/util/FunctionWrapper.h>

// TODO: complement ThreadContext and Thread. Thread supports to be derived. Use our special Thread instead of ThreadPool's std::thread. Then input thread info into Logger

LY_NAMESPACE_BEGIN
struct ThreadContext
{
  std::thread::id id;
  std::string name;

  explicit ThreadContext(std::string_view threadName)
    : name(threadName)
  {}

  void reset(std::string_view threadName)
  {
    id = std::thread::id{};
    name = threadName;
  }
};

class Thread
{
public:
  explicit Thread(const ThreadContext &context);
  ~Thread();

  template <typename Fn, typename... Args>
  auto dispatch(Fn&& fn, Args&&... args) -> std::future<std::invoke_result_t<Fn, Args...>>
  {
    using ResultType = std::invoke_result_t<Fn, Args...>;

    if (running_) this->destroy();

    auto task = std::make_shared<std::packaged_task<ResultType(Args...)>>(std::forward<Fn>(fn));
    auto res = task->get_future();

    running_ = true;
    thread_ = std::thread([task, &args...]{(*task)(std::forward<Args>(args)...);});
    this->fillContext();

    return res;
  }
  void destroy();

  ThreadContext context() const;

  LY_NONCOPYABLE(Thread);
private:
  void fillContext();

private:
  std::thread thread_;
  std::atomic_bool running_{false};

  ThreadContext context_;
};

LY_NAMESPACE_END
