#include <core/config/config.h>
#include <core/util/fiber/FiberScheduler.h>

LY_NAMESPACE_BEGIN
static auto g_max_fiber_capacity_per_scheduler = LY_CONFIG_GET(common.fiber.max_fiber_capacity_per_scheduler);

FiberScheduler::FiberScheduler()
{
  main_fiber_ = std::make_shared<Fiber>(this);
  ++s_total_count;
}

FiberScheduler::~FiberScheduler()
{
  RWMutex::wlock locker(rwmutex_);
  for (auto &[_, pFiber] : activate_list_) {
    // pFiber->yield();
    pFiber.reset();
  }
  --s_total_count;
}

void FiberScheduler::resume(FiberId fid)
{
  if (currentFiberId() == fid) return;

  RWMutex::wlock locker(rwmutex_);
  if (auto it = activate_list_.find(fid);
      it != activate_list_.end())
  {
    this->yield();
    it->second->resume();
  }
}
void FiberScheduler::yield()
{
  RWMutex::wlock locker(rwmutex_);
  if (main_fiber_->id() == -1) return;

  main_fiber_->yield();
}

auto FiberScheduler::track(std::shared_ptr<Fiber> pFiber) -> bool
{
  if (activate_list_.size() >= g_max_fiber_capacity_per_scheduler) {
    return false;
  }

  if (pFiber && pFiber->getStatus() == FiberStatus::RUNNABLE) {
    RWMutex::wlock locker(rwmutex_);
    if (auto it = activate_list_.find(pFiber->id());
        it == activate_list_.end())
    {
      activate_list_.emplace(pFiber->id(), pFiber);
      return true;
    }
  }

  return false;
}
auto FiberScheduler::untrack(FiberId fid) -> bool
{
  RWMutex::wlock locker(rwmutex_);
  if (auto it = activate_list_.find(fid);
      it != activate_list_.end())
  {
    activate_list_.erase(it);
    return true;
  }

  return false;
}

auto FiberScheduler::currentFiberId() const -> FiberId
{
  RWMutex::rlock locker(rwmutex_);
  return main_fiber_->id();
}
auto FiberScheduler::currentFiber() const -> Fiber::ptr
{
  RWMutex::rlock locker(rwmutex_);
  return main_fiber_;
}
auto FiberScheduler::count() const -> size_t
{
  RWMutex::rlock locker(rwmutex_);
  return activate_list_.size();
}

auto FiberScheduler::totalCount() -> uint64_t { return s_total_count; }

LY_NAMESPACE_END
