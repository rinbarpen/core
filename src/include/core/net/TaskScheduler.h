#pragma once
#include <atomic>
#include <functional>
#include <memory>

#include <core/util/marcos.h>
#include <core/net/FdChannel.h>
#include <core/net/Pipe.h>
#include <core/util/timer/Timer.h>
#include <core/util/buffer/RingBuffer.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

using TaskSchedulerId = std::string;
using TriggerEvent = std::function<void()>;
class TaskScheduler
{
protected:
	static const char kTriggetEvent = 1;
	static const char kTimerEvent = 2;
	static const int  kMaxTriggetEvents = 50000;
public:
  SHARED_REG(TaskScheduler);

	TaskScheduler(TaskSchedulerId id);
	virtual ~TaskScheduler() = default;

	void start();
	void stop();
  TimerTaskId addTimer(const TimerTask &task);
	void removeTimer(TimerTaskId timerId);
	bool addTriggerEvent(TriggerEvent callback);
  bool addTriggerEventForce(
    TriggerEvent callback, std::chrono::milliseconds timeout);

	TaskSchedulerId getId() const { return id_; }

	virtual void updateChannel(FdChannel::ptr channel) = 0;
	virtual void removeChannel(sockfd_t fd) = 0;
	virtual bool handleEvent(std::chrono::milliseconds timeout) = 0;

protected:
	void wake();
	void handleTriggerEvent();

	TaskSchedulerId id_;

	std::atomic_bool running_{ false };
	std::unique_ptr<Pipe> wakeup_pipe_;
	std::shared_ptr<FdChannel> wakeup_channel_;
	std::unique_ptr<RingBuffer<TriggerEvent>> trigger_events_;

	Mutex::type mutex_;
	std::unique_ptr<Timer> timers_;
};


NAMESPACE_END(net)
LY_NAMESPACE_END
