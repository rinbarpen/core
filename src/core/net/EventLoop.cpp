#include <memory>
#include <core/util/thread/ThreadPool.h>
#include <core/net/EventLoop.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
EventLoop::EventLoop(size_t nThreads)
  : capacity_(nThreads > 0 ? nThreads : 1)
{
  this->start();
}
EventLoop::~EventLoop()
{
  this->stop();
}

void EventLoop::start()
{
  pool_ = std::make_unique<ThreadPool>(capacity_);

  for (size_t i = 0; i < capacity_; ++i) {
#ifdef __LINUX__
    auto pTaskScheduler = EpollTaskScheduler::make_shared(std::to_string(i));
#elif defined(__WIN__)
    auto pTaskScheduler = SelectTaskScheduler::make_shared(std::to_string(i));
#endif
    task_schedulers_.push_back(pTaskScheduler);
    (void)pool_->submit(&TaskScheduler::start, pTaskScheduler.get());
  }
}
void EventLoop::stop()
{
  for (auto task_scheduler : task_schedulers_) {
    task_scheduler->stop();
  }
  pool_->stop();

  task_schedulers_.clear();
  pool_ = nullptr;
}

auto EventLoop::getTaskScheduler() -> TaskScheduler::ptr
{
	Mutex::lock locker(mutex_);
  auto task_scheduler = task_schedulers_[current_task_scheduler_];
  current_task_scheduler_ = (current_task_scheduler_ + 1) % task_schedulers_.size();
  return task_scheduler;
}

bool EventLoop::addTriggerEvent(TriggerEvent callback)
{
	Mutex::lock locker(mutex_);
  return task_schedulers_[current_task_scheduler_]->addTriggerEvent(callback);
}
bool EventLoop::addTriggerEventForce(TriggerEvent callback, std::chrono::milliseconds timeout)
{
	Mutex::lock locker(mutex_);
  return task_schedulers_[current_task_scheduler_]->addTriggerEventForce(callback, timeout);
}
TimerTaskId EventLoop::addTimer(TimerTask timerTask)
{
	Mutex::lock locker(mutex_);
  return task_schedulers_[current_task_scheduler_]->addTimer(timerTask);
}
void EventLoop::removeTimer(TimerTaskId timerTaskId)
{
	Mutex::lock locker(mutex_);
  task_schedulers_[current_task_scheduler_]->removeTimer(timerTaskId);
}
void EventLoop::updateChannel(FdChannel::ptr pChannel)
{
	Mutex::lock locker(mutex_);
  task_schedulers_[current_task_scheduler_]->updateChannel(pChannel);
}
void EventLoop::removeChannel(sockfd_t sockfd)
{
	Mutex::lock locker(mutex_);
  task_schedulers_[current_task_scheduler_]->removeChannel(sockfd);
}
NAMESPACE_END(net)
LY_NAMESPACE_END
