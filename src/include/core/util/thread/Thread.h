#pragma once

#include <atomic>
#include <thread>

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
  void dispatch(Fn&& fn, Args&&... args)
  {
    if (running_) return;

    running_ = true;
    thread_ = std::thread(std::forward<Fn>(fn), std::forward<Args>(args)...);
    this->fillContext();
  }
  template <typename Fn, typename... Args>
  void dispatch(FunctionWrapper<Fn, Args...>&& fn)
  {
    if (running_) return;

    running_ = true;
    thread_ = std::thread([=]() {
      (void)fn();
    });
    this->fillContext();
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
