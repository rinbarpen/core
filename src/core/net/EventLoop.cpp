#include <core/net/EventLoop.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_net_logger = GET_LOGGER("net");

EventLoop::EventLoop(EventLoopId id) :
  id_(id),
	wakeup_pipe_(new Pipe()),
	trigger_events_(new RingBuffer<TriggerEvent>(kMaxTriggetEvents))
{
	if (wakeup_pipe_->create()) {
		wakeup_channel_.reset(new FdChannel(wakeup_pipe_->readFd()));
		wakeup_channel_->enableReading();
		wakeup_channel_->setReadCallback([this]() { this->wake(); });
	}
}

void EventLoop::start()
{
	if (running_) return;

	ILOG_INFO(g_net_logger) << "EventLoop is starting...";
  running_ = true;

	while (running_) {
		this->handleTriggerEvent();
	  this->timers_.run();
    auto timeout = timers_.nextTimestamp();
		ILOG_TRACE(g_net_logger) << "timeout(ms): " << timeout.count();
	  this->handleEvent(timeout.duration());
	}
}

void EventLoop::stop()
{
	if (!running_) return;

	running_ = false;
	char event = kTriggetEvent;
	wakeup_pipe_->write(&event, 1);
}

auto EventLoop::addTimer(TimerTask task) -> TimerTaskId
{
	Mutex::lock locker(mutex_);
  if (timers_.add(task)) {
    return task.id;
  }

	return kInvalidTimerId;
}

void EventLoop::removeTimer(TimerTaskId timerId)
{
	Mutex::lock locker(mutex_);
	timers_.remove(timerId);
}

bool EventLoop::addTriggerEvent(TriggerEvent callback)
{
	if (trigger_events_->size() < kMaxTriggetEvents) {
		Mutex::lock locker(mutex_);
		trigger_events_->push(callback);
		char event = EventLoop::kTriggetEvent;
		wakeup_pipe_->write(&event, 1);
		return true;
	}

	return false;
}

bool EventLoop::addTriggerEventForce(TriggerEvent cb, std::chrono::milliseconds timeout)
{
	if (trigger_events_->size() < kMaxTriggetEvents) {
		Mutex::lock locker(mutex_);
		trigger_events_->push(cb);
		// TODO:
		char event = kTriggetEvent;
		wakeup_pipe_->write(&event, 1);
		return true;
	}

	return kInvalidTimerId != this->addTimer(Timer::newTask([cb] {
		cb();
	}, timeout));
}

NAMESPACE_END(net)
LY_NAMESPACE_END
