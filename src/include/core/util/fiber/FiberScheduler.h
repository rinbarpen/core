#pragma once

#include <memory>
#include <queue>
#include <unordered_map>

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/util/Singleton.h>
#include <core/util/fiber/FiberDefine.h>
#include <core/util/fiber/Fiber.h>


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

  static auto totalCount() -> uint64_t;
  static auto newFiber() -> std::shared_ptr<Fiber>;

  LY_NONCOPYABLE(FiberScheduler);
private:
  std::shared_ptr<Fiber> main_fiber_;

  std::unordered_map<FiberId, std::shared_ptr<Fiber>> activate_list_;
  std::queue<std::shared_ptr<Fiber>> free_list_;

  mutable RWMutex::type rwmutex_;

  static inline uint64_t s_total_count{0};
};

using MainFiberScheduler = Singleton<FiberScheduler>;

LY_NAMESPACE_END
