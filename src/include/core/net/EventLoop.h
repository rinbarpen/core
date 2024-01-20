#pragma once

#include <core/util/thread/ThreadPool.h>
#include <core/net/EpollTaskScheduler.h>
#include <core/net/SelectTaskScheduler.h>
#include <vector>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class EventLoop
{
public:
  EventLoop(size_t nThreads = std::thread::hardware_concurrency());
  virtual ~EventLoop();

  void start();
  void stop();

  auto getTaskScheduler() -> TaskScheduler::ptr;

	bool addTriggerEvent(TriggerEvent callback);
  bool addTriggerEventForce(TriggerEvent callback, std::chrono::milliseconds timeout);
	TimerTaskId addTimer(TimerTask timerTask);
	void removeTimer(TimerTaskId timerTaskId);
	void updateChannel(FdChannel::ptr pChannel);
	void removeChannel(sockfd_t sockfd);

  LY_NONCOPYABLE(EventLoop);

private:
  std::unique_ptr<ThreadPool> pool_;
  std::vector<TaskScheduler::ptr> task_schedulers_;
  size_t current_task_scheduler_{0};
  size_t capacity_;

  mutable Mutex::type mutex_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
