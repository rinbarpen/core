#include <core/util/thread/Thread.h>

LY_NAMESPACE_BEGIN
Thread::Thread(const ThreadContext &context)
  : context_(context)
{
}
Thread::Thread(std::string_view name)
  : context_(name)
{
}
Thread::~Thread()
{
  if (thread_.joinable()) {
    thread_.join();
  }
  running_ = false;
}
void Thread::start()
{
  if (running_) this->stop();

  running_ = true;
  thread_ = std::thread(&Thread::run, this);
  context_.reset(thread_.get_id());

  s_thread_mapping[thread_.get_id()] = context_;
}
void Thread::stop()
{
  if (!running_) return;

  s_thread_mapping.erase(thread_.get_id());
  thread_.join();
  context_.clear();

  running_ = false;
}

ThreadContext Thread::context() const
{
  return context_;
}

LY_NAMESPACE_END
