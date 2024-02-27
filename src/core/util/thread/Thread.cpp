#include <core/util/thread/Thread.h>

LY_NAMESPACE_BEGIN

Thread::Thread(const ThreadContext &context)
  : context_(context)
{}
Thread::Thread(std::string_view name)
  : context_(name)
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
  context_.clear();

  running_ = false;
}

ThreadContext Thread::context() const
{
  return context_;
}

LY_NAMESPACE_END
