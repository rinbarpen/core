#include <core/util/thread/Thread.h>

LY_NAMESPACE_BEGIN

Thread::Thread(const ThreadContext &context)
  : context_(context)
{}

Thread::~Thread()
{
  if (thread_.joinable()) {
    thread_.join();
  }
  running_ = false;
}
void Thread::destroy()
{
  if (!running_) return;

  thread_.join();
  context_.reset(context_.name);

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
