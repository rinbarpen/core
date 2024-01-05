#include <core/net/SelectEventLoop.h>
#include <forward_list>
#include <core/util/timer/Timer.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_net_logger = GET_LOGGER("net");

SelectEventLoop::SelectEventLoop(EventLoopId id) :
  EventLoop(id)
{
  FD_ZERO(&fd_read_backup_);
  FD_ZERO(&fd_write_backup_);
  FD_ZERO(&fd_exp_backup_);

  this->updateChannel(wakeup_channel_);
}
SelectEventLoop::~SelectEventLoop()
{
  
}

void SelectEventLoop::updateChannel(FdChannel::ptr pChannel)
{
	Mutex::lock locker(mutex_);

	sockfd_t fd = pChannel->getSockfd();

	if (channels_.find(fd) != channels_.end()) {
		if (pChannel->isNoneEvent()) {
			is_fd_read_reset_ = true;
			is_fd_write_reset_ = true;
			is_fd_exp_reset_ = true;
			channels_.erase(fd);
		}
		else {
			//is_fd_read_reset_ = true;
			is_fd_write_reset_ = true;
		}
	}
	else {
		if (!pChannel->isNoneEvent()) {
			channels_.emplace(fd, pChannel);
			is_fd_read_reset_ = true;
			is_fd_write_reset_ = true;
			is_fd_exp_reset_ = true;
		}
	}
}

void SelectEventLoop::removeChannel(sockfd_t fd)
{
	Mutex::lock locker(mutex_);

	if (channels_.find(fd) != channels_.end()) {
		is_fd_read_reset_ = true;
		is_fd_write_reset_ = true;
		is_fd_exp_reset_ = true;
		channels_.erase(fd);
	}
}

bool SelectEventLoop::handleEvent(std::chrono::milliseconds timeout)
{
  ILOG_DEBUG(g_net_logger) << "Handle events in SelectEventLoop";
  ILOG_DEBUG(g_net_logger) << "The count of event FdChannel is " << channels_.size();
	if (channels_.empty()) {
		ILOG_DEBUG(g_net_logger) << "Sleep " << timeout.count() << "(ms)";
    Timer::sleep(timeout);
		return true;
	}

	fd_set fd_read;
	fd_set fd_write;
	fd_set fd_exp;
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_exp);
	bool fd_read_reset = false;
	bool fd_write_reset = false;
	bool fd_exp_reset = false;

	if (is_fd_read_reset_ || is_fd_write_reset_ || is_fd_exp_reset_) {
		if (is_fd_exp_reset_) {
			max_fd_ = 0;
		}

		Mutex::lock locker(mutex_);
		for (auto &[_, pChannel] : channels_) {
			int events = pChannel->getEvents();
			sockfd_t fd = pChannel->getSockfd();

			if (is_fd_read_reset_ && (events & EVENT_IN)) {
				FD_SET(fd, &fd_read);
			}

			if (is_fd_write_reset_ && (events & EVENT_OUT)) {
				FD_SET(fd, &fd_write);
			}

			if (is_fd_exp_reset_) {
				FD_SET(fd, &fd_exp);
				if (fd > max_fd_) {
					max_fd_ = fd;
				}
			}
		}

		fd_read_reset = is_fd_read_reset_;
		fd_write_reset = is_fd_write_reset_;
		fd_exp_reset = is_fd_exp_reset_;
		is_fd_read_reset_ = false;
		is_fd_write_reset_ = false;
		is_fd_exp_reset_ = false;
	}

	if (fd_read_reset) {
		FD_ZERO(&fd_read_backup_);
		memcpy(&fd_read_backup_, &fd_read, sizeof(fd_set));
	}
	else {
		memcpy(&fd_read, &fd_read_backup_, sizeof(fd_set));
	}

	if (fd_write_reset) {
		FD_ZERO(&fd_write_backup_);
		memcpy(&fd_write_backup_, &fd_write, sizeof(fd_set));
	}
	else {
		memcpy(&fd_write, &fd_write_backup_, sizeof(fd_set));
	}

	if (fd_exp_reset) {
		FD_ZERO(&fd_exp_backup_);
		memcpy(&fd_exp_backup_, &fd_exp, sizeof(fd_set));
	}
	else {
		memcpy(&fd_exp, &fd_exp_backup_, sizeof(fd_set));
	}

	// timeout = 1000ms;
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
  auto microseconds =
    std::chrono::duration_cast<std::chrono::microseconds>(timeout - seconds);
  struct timeval tv;
  tv.tv_sec = seconds.count();
  tv.tv_usec = microseconds.count();

  int ret = ::select((int)max_fd_ + 1, &fd_read, &fd_write, &fd_exp, &tv);
  if (ret < 0) {
#if defined(__LINUX__)
		if (errno == EINTR) {
			return true;
		}
#endif
		ILOG_CRITICAL(g_net_logger) << "Fail!! select() return: " << ret;
		return false;
	}

	ILOG_DEBUG(g_net_logger) << "Notify!! select() return: " << ret;
	std::forward_list<std::pair<FdChannel::ptr, int>> event_list;
	if (ret > 0) {
		ILOG_DEBUG(g_net_logger) << "There is some events";
		Mutex::lock locker(mutex_);
		for (auto &[_, pChannel] : channels_) {
			int events = 0;
			sockfd_t fd = pChannel->getSockfd();

			if (FD_ISSET(fd, &fd_read)) {
				events |= EVENT_IN;
			}

			if (FD_ISSET(fd, &fd_write)) {
				events |= EVENT_OUT;
			}

			if (FD_ISSET(fd, &fd_exp)) {
				events |= (EVENT_HUP); // close
			}

			if (events != 0) {
				event_list.emplace_front(pChannel, events);
			}
		}
	}

	for (auto &[pChannel, events] : event_list) {
		ILOG_INFO(g_net_logger) << "Handle Event: " << pChannel->getSockfd() << ":" << events;
		pChannel->handleEvent(events);
	}

	return true;
}

NAMESPACE_END(net)
LY_NAMESPACE_END
