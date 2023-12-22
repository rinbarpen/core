#include <core/util/fiber/FiberScheduler.h>

LY_NAMESPACE_BEGIN
FiberScheduler::FiberScheduler()
{
  main_fiber_ = std::make_shared<Fiber>(this);
}

FiberScheduler::~FiberScheduler()
{
  RWMutex::wlock locker(rwmutex_);
  for (auto &[_, pFiber] : fibers_) {
    pFiber->yield();
    pFiber.reset();
  }
}

void FiberScheduler::resume(FiberId fid)
{
  if (currentFiberId() == fid) return;

  RWMutex::wlock locker(rwmutex_);
  if (auto it = fibers_.find(fid); 
      it != fibers_.end())
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
  if (pFiber && pFiber->getStatus() == FiberStatus::RUNNABLE) {
    RWMutex::wlock locker(rwmutex_);
    if (auto it = fibers_.find(pFiber->id()); it == fibers_.end())
    {
      fibers_.emplace(pFiber->id(), pFiber);
      return true;
    }
  }

  return false;
}
auto FiberScheduler::untrack(FiberId fid) -> bool
{
  RWMutex::wlock locker(rwmutex_);
  if (auto it = fibers_.find(fid); 
      it != fibers_.end())
  {
    fibers_.erase(it);
    locker.unlock();
    return true;
  }

  locker.unlock();
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
  return fibers_.size();
}



LY_NAMESPACE_END
