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

using EventLoopId = std::string;
using TriggerEvent = std::function<void()>;
class EventLoop
{
protected:
	static const char kTriggetEvent = 1;
	static const char kTimerEvent = 2;
	static const int  kMaxTriggetEvents = 50000;
public:
  SHARED_REG(EventLoop);

	EventLoop(EventLoopId id);
	virtual ~EventLoop() = default;

	void start();
	void stop();
  auto addTimer(TimerTask task) -> TimerTaskId;
	void removeTimer(TimerTaskId timerId);
	auto addTriggerEvent(TriggerEvent callback) -> bool;
  auto addTriggerEventForce(
    TriggerEvent callback, std::chrono::milliseconds timeout) -> bool;

	auto getId() const -> EventLoopId { return id_; }

	virtual void updateChannel(FdChannel::ptr channel) { }
	virtual void removeChannel(sockfd_t fd) { }
	virtual bool handleEvent(std::chrono::milliseconds timeout) { return false; }

protected:
	void wake()
	{
		char event[10] = { 0 };
		while (wakeup_pipe_->read(event, 10) > 0);
	}
	void handleTriggerEvent()
	{
		do {
			TriggerEvent callback;
			if (trigger_events_->pop(callback)) {
				callback();
			}
		} while (!trigger_events_->empty());
	}

	EventLoopId id_;

	std::atomic_bool running_{ false };
	std::unique_ptr<Pipe> wakeup_pipe_;
	std::shared_ptr<FdChannel> wakeup_channel_;
	std::unique_ptr<RingBuffer<TriggerEvent>> trigger_events_;

	Mutex::type mutex_;
	Timer timers_;
};


NAMESPACE_END(net)
LY_NAMESPACE_END
