#include <core/net/TaskScheduler.h>
#include <core/util/Mutex.h>

#if defined(__LINUX__)
# include <signal.h>
#endif

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_net_logger = GET_LOGGER("net");

TaskScheduler::TaskScheduler(TaskSchedulerId id) :
  id_(id),
	wakeup_pipe_(new Pipe()),
	trigger_events_(new RingBuffer<TriggerEvent>(kMaxTriggetEvents)),
	timers_(new Timer())
{
	if (wakeup_pipe_->create()) {
		wakeup_channel_.reset(new FdChannel(wakeup_pipe_->readFd()));
		wakeup_channel_->enableReading();
		wakeup_channel_->setReadCallback([this]() { this->wake(); });
	}
}

void TaskScheduler::start()
{
	if (running_) return;

#if defined(__LINUX__)
	signal(SIGPIPE, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGKILL, SIG_IGN);
#endif

	ILOG_INFO(g_net_logger) << "TaskScheduler is starting...";
  running_ = true;

	while (running_) {
		this->handleTriggerEvent();
	  this->timers_->run();
    auto timeout = timers_->nextTimestamp();
		// ILOG_TRACE(g_net_logger) << "timeout(ms): " << timeout.count();
	  this->handleEvent(timeout.duration());
	}
}

void TaskScheduler::stop()
{
	if (!running_) return;

	running_ = false;
	char event = kTriggetEvent;
	wakeup_pipe_->write(&event, 1);
}

TimerTaskId TaskScheduler::addTimer(const TimerTask &task)
{
	Mutex::lock locker(mutex_);
  if (timers_->add(task)) {
    return task.id;
  }

	return kInvalidTimerId;
}

void TaskScheduler::removeTimer(TimerTaskId timerId)
{
	Mutex::lock locker(mutex_);
	timers_->remove(timerId);
}

bool TaskScheduler::addTriggerEvent(TriggerEvent callback)
{
	if (trigger_events_->size() < kMaxTriggetEvents) {
		Mutex::lock locker(mutex_);
		trigger_events_->push(callback);
		char event = TaskScheduler::kTriggetEvent;
		wakeup_pipe_->write(&event, 1);
		return true;
	}

	return false;
}

bool TaskScheduler::addTriggerEventForce(TriggerEvent callback, std::chrono::milliseconds timeout)
{
	if (trigger_events_->size() < kMaxTriggetEvents) {
		Mutex::lock locker(mutex_);
		trigger_events_->push(callback);

		char event = kTriggetEvent;
		wakeup_pipe_->write(&event, 1);
		return true;
	}

	return kInvalidTimerId != this->addTimer(Timer::newTask([callback] {
		callback();
	}, timeout));
}

void TaskScheduler::wake()
{
	char event[10] = { 0 };
	while (wakeup_pipe_->read(event, 10) > 0);
}
void TaskScheduler::handleTriggerEvent()
{
	do {
		TriggerEvent callback;
		if (trigger_events_->pop(callback)) {
			callback();
		}
	} while (!trigger_events_->empty());
}

NAMESPACE_END(net)
LY_NAMESPACE_END
