#include <core/util/thread/Thread.h>

LY_NAMESPACE_BEGIN

Thread::Thread()
{
}

Thread::~Thread()
{
  if (thread_.joinable()) {
    thread_.join();
  }
  running_ = false;
}

template <typename Fn, typename... Args>
void Thread::dispatch(Fn &&fn, Args &&...args) 
{
  if (running_) return;

  running_ = true;
  thread_ = std::thread(std::forward<Fn>(fn), std::forward<Args>(args...));
  this->fillContext();
}

template <typename Fn, typename ... Args>
void Thread::dispatch(FunctionWrapper<Fn, Args...>&& fn)
{
  if (running_) return;

  running_ = true;
  thread_ = std::thread([=]() {
    (void)fn();
  });
  this->fillContext();
}

void Thread::destroy()
{
  if (!running_) return;

  thread_.join();
  context_.reset();

  running_ = false;
}

ThreadContext Thread::context() const
{
  return context_;
}

void Thread::fillContext()
{
  context_.id = std::thread::id{};

}

LY_NAMESPACE_END
