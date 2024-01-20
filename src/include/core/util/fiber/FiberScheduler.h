#pragma once

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/util/fiber/FiberConstant.h>
#include <core/util/fiber/Fiber.h>

#include <unordered_map>

LY_NAMESPACE_BEGIN

class Fiber;

class FiberScheduler
{
  friend class Fiber;
public:
  FiberScheduler();
  ~FiberScheduler();

  void resume(FiberId fid);
  void yield();

  auto track(std::shared_ptr<Fiber> pFiber) -> bool;
  auto untrack(FiberId fid) -> bool;

  auto currentFiberId() const -> FiberId;
  auto currentFiber() const -> std::shared_ptr<Fiber>;

  auto count() const -> size_t;

  LY_NONCOPYABLE(FiberScheduler);
private:
  std::shared_ptr<Fiber> main_fiber_;

  std::unordered_map<FiberId, std::shared_ptr<Fiber>> fibers_;

  mutable RWMutex::type rwmutex_;
};

LY_NAMESPACE_END
