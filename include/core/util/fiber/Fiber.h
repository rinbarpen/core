#pragma once

#include <functional>
#include <utility>
#include <core/util/fiber/FiberConstant.h>
#include <core/util/fiber/FiberScheduler.h>

#include <ucontext.h>

LY_NAMESPACE_BEGIN

enum class FiberStatus
{
  NONE,     // uninitialized
  RUNNABLE, // initialized
  RUNNING,  // from RUNABLE(first) or SUSPEND
  SUSPEND,  // from RUNNING or RUNABLE
  DEAD,     // complete, from SUSPEND
};
struct FiberContext
{
  int buffer_size{0};
  char *buffer{};
#ifdef __LINUX__
  ucontext_t ctx;
#endif
  FiberStatus status{FiberStatus::NONE};
};

class FiberScheduler;
class Fiber
{
  friend class FiberScheduler;
public:

  SHARED_PTR_USING(Fiber, ptr);

  using InitializedCallback = std::function<void()>;
  using DestroyedCallback = std::function<void()>;
  using RunningCallback = std::function<void()>;
  using YieldCallback = std::function<void()>;

  Fiber(FiberScheduler *pScheduler);
  Fiber(InitializedCallback initializedCallback,
        RunningCallback runningCallback,
        YieldCallback yieldCallback,
        DestroyedCallback destroyedCallback,
        FiberScheduler *pScheduler);
  virtual ~Fiber();

  static Fiber::ptr newFiber(FiberScheduler *pScheduler)
  {
    return std::make_shared<Fiber>(pScheduler);
  }
  static Fiber::ptr newFiber(InitializedCallback initializedCallback,
    RunningCallback runningCallback, YieldCallback yieldCallback,
    DestroyedCallback destroyedCallback,
    FiberScheduler *pScheduler)
  {
    auto pFiber = std::make_shared<Fiber>(pScheduler);
    pFiber->set(std::move(initializedCallback),
                std::move(runningCallback),
                std::move(yieldCallback),
                std::move(destroyedCallback));
    return pFiber;
  }

  void resume();
  void yield();

  virtual void run() {}

  // Callbacks
  void set(InitializedCallback initializedCallback,
           RunningCallback runningCallback,
           YieldCallback yieldCallback,
           DestroyedCallback destroyedCallback);
  void setInitializedCallback(InitializedCallback callback);
  void setRunningCallback(RunningCallback callback);
  void setYieldCallback(YieldCallback callback);
  void setDestroyedCallback(DestroyedCallback callback);

  void setStatus(FiberStatus status) { context_.status = status; }
  auto getStatus() const -> FiberStatus { return context_.status; }
  auto id() const -> FiberId { return reinterpret_cast<uintptr_t>(this) & 0xFFFF; }

  LY_NONCOPYABLE(Fiber);

protected:
  virtual void ready() const { initialized_callback_(); }
  virtual void running() const { running_callback_(); }
  virtual void dead() const { destroyed_callback_(); }

private:
  void saveStack();
  void loadStack();

  void init();

protected:
  FiberContext context_;

  InitializedCallback initialized_callback_{};
  RunningCallback running_callback_{};
  YieldCallback yield_callback_{};
  DestroyedCallback destroyed_callback_{};

private:
  FiberScheduler *scheduler_{ nullptr };
};

LY_NAMESPACE_END
