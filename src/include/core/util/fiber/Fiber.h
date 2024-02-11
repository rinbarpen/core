#pragma once

#include <cstdint>
#include <functional>
#include <utility>
#include <core/util/fiber/FiberConstant.h>
#include <core/util/fiber/FiberScheduler.h>

#include <ucontext.h>

// TODO: Add this to EpollTaskScheduler.
// TODO: Test me!
// TODO: add io_uring to play.

LY_NAMESPACE_BEGIN

enum class FiberStatus
{
  NONE,     // uninitialized
  RUNNABLE, // initialized
  RUNNING,  // from RUNNABLE(first) or SUSPEND
  SUSPEND,  // from RUNNING
  DEAD,     // complete, from SUSPEND
  ABNORMAL, // exception or abort
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

  using RunningCallback = std::function<void()>;

  Fiber(FiberScheduler *pScheduler);
  Fiber(RunningCallback callback,
        FiberScheduler *pScheduler);
  virtual ~Fiber();

  static Fiber::ptr newFiber(FiberScheduler *pScheduler)
  {
    return std::make_shared<Fiber>(pScheduler);
  }
  static Fiber::ptr newFiber(RunningCallback callback,
    FiberScheduler *pScheduler)
  {
    auto pFiber = std::make_shared<Fiber>(pScheduler);
    pFiber->reset(callback);
    return pFiber;
  }

  void resume();
  void yield();

  static void entry(void *pFiber);

  // Callbacks
  void reset(RunningCallback callback);

  void setStatus(FiberStatus status) { context_.status = status; }
  auto getStatus() const -> FiberStatus { return context_.status; }
  auto id() const -> FiberId { return reinterpret_cast<uintptr_t>(this) & 0xFFFF; }

  static auto totalCount() -> uint64_t;

  LY_NONCOPYABLE(Fiber);

private:
  void saveStack();
  void loadStack();

  void init();

protected:
  FiberContext context_;


private:
  FiberScheduler *scheduler_{ nullptr };
  RunningCallback callback_{};

  static inline uint64_t s_total_count{0};
};

LY_NAMESPACE_END
