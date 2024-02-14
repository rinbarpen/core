#include <cstring>
#include <exception>

#include <core/util/Util.h>
#include <core/config/config.h>
#include <core/util/logger/Logger.h>
#include <core/util/fiber/Fiber.h>

#include <fmt/core.h>

// FIXME: How to switch to DEAD status

LY_NAMESPACE_BEGIN
static auto g_fiber_logger = GET_LOGGER("system.fiber");
static auto g_max_fiber_buffer_size = LY_CONFIG_GET(common.fiber.max_buffer_size);
static auto g_max_fiber_capacity_per_scheduler = LY_CONFIG_GET(common.fiber.max_fiber_capacity_per_scheduler);

Fiber::Fiber(FiberScheduler* scheduler)
  : scheduler_(scheduler)
{
  LY_ASSERT(scheduler_);
  context_.buffer = new char[g_max_fiber_buffer_size];
  init();
}
Fiber::Fiber(RunningCallback callback, FiberScheduler* scheduler)
  : scheduler_(scheduler), callback_(callback)
{
  LY_ASSERT(scheduler_);
  context_.buffer = new char[g_max_fiber_buffer_size];
  init();
}

Fiber::~Fiber()
{
  LY_ASSERT2(context_.status != FiberStatus::RUNNING, fmt::format("Fiber({}) should be joined", id()));
  --s_total_count;
  delete[] context_.buffer;
}

Fiber::ptr Fiber::newFiber(FiberScheduler *scheduler)
{
  if (!scheduler) {
    scheduler = MainFiberScheduler::instance();
  }

  if (scheduler->count() >= g_max_fiber_capacity_per_scheduler) {
    return nullptr;
  }
  return std::make_shared<Fiber>(scheduler);
}
Fiber::ptr Fiber::newFiber(RunningCallback callback,
  FiberScheduler *scheduler)
{
  if (!scheduler) {
    scheduler = MainFiberScheduler::instance();
  }

  if (scheduler->count() >= g_max_fiber_capacity_per_scheduler) {
    return nullptr;
  }
  return std::make_shared<Fiber>(callback, scheduler);
}


void Fiber::reset(RunningCallback callback)
{
  LY_ASSERT2(context_.status == FiberStatus::DEAD
         || context_.status == FiberStatus::RUNNABLE
         || context_.status == FiberStatus::ABNORMAL,
         fmt::format("The status of fiber({}) is {}, wrong status", id(), static_cast<int>(getStatus())));
  callback_ = callback;

  ::getcontext(&context_.ctx);
  context_.ctx.uc_link = scheduler_ ? &scheduler_->main_fiber_->context_.ctx : nullptr;
  context_.ctx.uc_stack.ss_flags = 0;
  context_.ctx.uc_stack.ss_size = g_max_fiber_buffer_size;
  context_.ctx.uc_stack.ss_sp = &context_.buffer;

  ::makecontext(&context_.ctx, (void(*)())&Fiber::entry, 1,
    static_cast<void *>(this));
}

void Fiber::resume()
{
  switch (context_.status) {
  case FiberStatus::RUNNABLE:
  {
    ::swapcontext(&scheduler_->main_fiber_->context_.ctx, &context_.ctx);
    context_.status = FiberStatus::RUNNING;
    break;
  }
  case FiberStatus::SUSPEND:
  {
    loadStack();
    ::swapcontext(&scheduler_->main_fiber_->context_.ctx, &context_.ctx);
    context_.status = FiberStatus::RUNNING;
    break;
  }
  default: break;
  } // switch
}
void Fiber::yield()
{
  switch (context_.status) {
  case FiberStatus::RUNNING:
    context_.status = FiberStatus::SUSPEND;
    saveStack();
    ::swapcontext(&context_.ctx, &scheduler_->main_fiber_->context_.ctx);
    break;
  default: break;
  }
}

void Fiber::saveStack()
{
  char *p = scheduler_->main_fiber_->context_.buffer
            + g_max_fiber_buffer_size;
  char dummy = 0;

  int used = p - &dummy;
  LY_ASSERT(used <= g_max_fiber_buffer_size);
  context_.buffer_size = used;

  ::memcpy(context_.buffer, &dummy, used);
}
void Fiber::loadStack()
{
  void *p = scheduler_->main_fiber_->context_.buffer
            + g_max_fiber_buffer_size - scheduler_->main_fiber_->context_.buffer_size;
  ::memcpy(p, context_.buffer, context_.buffer_size);
}

auto Fiber::totalCount() -> uint64_t { return s_total_count; }

void Fiber::entry(void *pFiber)
{
  auto *co = static_cast<Fiber *>(pFiber);
  LY_ASSERT(co->getStatus() != FiberStatus::ABNORMAL || co->getStatus() != FiberStatus::NONE);
  if (co->getStatus() == FiberStatus::DEAD) { return ; }

  co->setStatus(FiberStatus::RUNNING);
  try {
    co->callback_();
  } catch(std::exception &ex) {
    co->context_.status = FiberStatus::ABNORMAL;
    ILOG_ERROR(g_fiber_logger) << ex.what() << " with fiber id = " << co->id();
  } catch(...) {
    co->context_.status = FiberStatus::ABNORMAL;
    ILOG_ERROR(g_fiber_logger) << "unknown exception with fiber id = " << co->id();
  }
  co->yield();
}


void Fiber::init()
{
  ::getcontext(&context_.ctx);
  context_.ctx.uc_link = scheduler_ ? &scheduler_->main_fiber_->context_.ctx : nullptr;
  context_.ctx.uc_stack.ss_flags = 0;
  context_.ctx.uc_stack.ss_size = g_max_fiber_buffer_size;
  context_.ctx.uc_stack.ss_sp = &context_.buffer;

  ::makecontext(&context_.ctx, (void(*)())&Fiber::entry, 1,
    static_cast<void *>(this));
  ++s_total_count;
}

LY_NAMESPACE_END
