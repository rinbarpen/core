#pragma once

#include <atomic>
#include <thread>

#include <core/util/marcos.h>
#include <core/util/FunctionWrapper.h>

LY_NAMESPACE_BEGIN

struct ThreadContext
{
  std::thread::id id;
  std::string name;


  void reset()
  {
    id = std::thread::id{};
    name = "Unknown";
  }
};

class Thread
{
public:
  Thread();
  ~Thread();

  template <typename Fn, typename... Args>
  void dispatch(Fn&& fn, Args&&... args);
  template <typename Fn, typename... Args>
  void dispatch(FunctionWrapper<Fn, Args...>&& fn);
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
