#include <core/util/fiber/Fiber.h>

LY_NAMESPACE_BEGIN
Fiber::Fiber(FiberScheduler* pSchduler)
  : scheduler_(pSchduler)
{
  LY_ASSERT(scheduler_);
  context_.buffer = new char[kDefaultMaxFiberBufferSize];
  init();
}

Fiber::Fiber(InitializedCallback initializedCallback, 
  RunningCallback runningCallback, 
  YieldCallback yieldCallback,
  DestroyedCallback destroyedCallback, 
  FiberScheduler* pSchduler)
  : scheduler_(pSchduler)
{
  LY_ASSERT(scheduler_);
  this->set(std::move(initializedCallback), std::move(runningCallback),
            std::move(yieldCallback), std::move(destroyedCallback));
  context_.buffer = new char[kDefaultMaxFiberBufferSize];
  init();
}

Fiber::~Fiber()
{
  destroyed_callback_();
  delete[] context_.buffer;
}


void Fiber::set(InitializedCallback initializedCallback,
  RunningCallback runningCallback, 
  YieldCallback yieldCallback,
  DestroyedCallback destroyedCallback)
{
  initialized_callback_ = initializedCallback;
  running_callback_ = runningCallback;
  yield_callback_ = yieldCallback;
  destroyed_callback_ = destroyedCallback;
}

void Fiber::setInitializedCallback(InitializedCallback callback)
{
  initialized_callback_ = callback;
}

void Fiber::setRunningCallback(RunningCallback callback)
{
  running_callback_ = callback;
}

void Fiber::setYieldCallback(YieldCallback callback)
{
  yield_callback_ = callback;
}

void Fiber::setDestroyedCallback(DestroyedCallback callback)
{
  destroyed_callback_ = callback;
}

void Fiber::resume()
{
  switch (context_.status) {
  case FiberStatus::RUNNABLE:
  {
#ifdef __LINUX__
    ::swapcontext(&scheduler_->main_fiber_->context_.ctx, &context_.ctx);
#endif
    context_.status = FiberStatus::RUNNING;
    running_callback_();
    break;
  }
  case FiberStatus::SUSPEND:
  {
    loadStack();
#ifdef __LINUX__
    ::swapcontext(&scheduler_->main_fiber_->context.ctx, &context_.ctx);
#endif
    context_.status = FiberStatus::RUNNING;
    running_callback_();
    break;
  }
  } // switch
}
void Fiber::yield()
{
  switch (context_.status) {
  case FiberStatus::RUNNING:
    context_.status = FiberStatus::SUSPEND;
    saveStack();
#ifdef __LINUX__
    ::swapcontext(&context_.ctx, &scheduler_->main_fiber_->context.ctx);
#endif
    yield_callback_();
    break;
  }
}

void Fiber::saveStack()
{
  char *p = scheduler_->main_fiber_->context_.buffer
            + kDefaultMaxFiberBufferSize;
  char dummy = 0;

  int used = p - &dummy;
  LY_ASSERT(used <= kDefaultMaxFiberBufferSize);
  context_.buffer_size = used;

  ::memcpy(context_.buffer, &dummy, used);
}
void Fiber::loadStack()
{
  void *p = scheduler_->main_fiber_->context_.buffer
            + kDefaultMaxFiberBufferSize - scheduler_->main_fiber_->context_.buffer_size;
  ::memcpy(p, 
    scheduler_->main_fiber_->context_.buffer,
    scheduler_->main_fiber_->context_.buffer_size);
}

static void fiber_entry(void *pFiber)
{
  auto *co = static_cast<Fiber *>(pFiber);
  co->setStatus(FiberStatus::RUNNING);
  co->run();
  co->setStatus(FiberStatus::SUSPEND);
}


void Fiber::init()
{
#ifdef __LINUX__
  ::getcontext(&context_.ctx);
  context_.ctx.uc_link = &scheduler_->main_fiber_->context_.ctx;
  context_.ctx.uc_stack.ss_flags = 0;
  context_.ctx.uc_stack.ss_size = kDefaultMaxFiberBufferSize;
  context_.ctx.uc_stack.ss_sp = &scheduler_->main_fiber_->context_.buffer;

  ::makecontext(&context_.ctx, (void (*)()) & fiber_entry, 1,
    static_cast<void *>(this));
#endif
  initialized_callback_();
}

LY_NAMESPACE_END
