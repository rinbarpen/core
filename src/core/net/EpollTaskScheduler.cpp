#include <core/net/FdChannel.h>
#include <core/net/platform.h>
#include <core/util/logger/Logger.h>
#include <core/net/EpollTaskScheduler.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_net_logger = GET_LOGGER("net");

EpollTaskScheduler::EpollTaskScheduler(TaskSchedulerId id)
  : TaskScheduler(id)
{
#if defined(__LINUX__)
  epfd_ = ::epoll_create1(0);
  if (epfd_ < 0) {
    ILOG_ERROR(g_net_logger) << "Fail to create Epoll";
  }
#endif
  this->updateChannel(wakeup_channel_);
}

EpollTaskScheduler::~EpollTaskScheduler()
{
#if defined(__LINUX__)
  if (epfd_ > 0) ::close(epfd_);
#endif
}

void EpollTaskScheduler::control(int op, FdChannel::ptr pChannel)
{
#if defined(__LINUX__)
	struct epoll_event event = {0};

	if(op != EPOLL_CTL_DEL) {
		event.data.ptr = pChannel.get();
		event.events = pChannel->getEvents();
	}

	if(::epoll_ctl(epfd_, op, pChannel->getSockfd(), &event) < 0) {
    ILOG_WARN_FMT(g_net_logger, "Fail to set {} for {}", op, pChannel->getSockfd());
	}
#endif
}
void EpollTaskScheduler::updateChannel(FdChannel::ptr pChannel)
{
  Mutex::lock locker(mutex_);

  sockfd_t fd = pChannel->getSockfd();
	if(channels_.find(fd) != channels_.end()) {
		if(pChannel->isNoneEvent()) {
			this->control(EPOLL_CTL_DEL, pChannel);
			channels_.erase(fd);
		}
		else {
			this->control(EPOLL_CTL_MOD, pChannel);
		}
	}
	else {
		if(!pChannel->isNoneEvent()) {
			channels_.emplace(fd, pChannel);
			this->control(EPOLL_CTL_ADD, pChannel);
		}
	}
}
void EpollTaskScheduler::removeChannel(sockfd_t sockfd)
{
  Mutex::lock locker(mutex_);
  if (auto it = channels_.find(sockfd);
      it != channels_.end()) {
    this->control(EPOLL_CTL_DEL, it->second);
    channels_.erase(it);
  }
}
bool EpollTaskScheduler::handleEvent(std::chrono::milliseconds timeout)
{
#if defined(__LINUX__)
  // TODO: Use Config instead
  const int MAX_EVENTS = 100;
  struct epoll_event events[MAX_EVENTS]{0};

  Mutex::lock locker(mutex_);
  int num_events = ::epoll_wait(epfd_, events, MAX_EVENTS, timeout.count());
  if (num_events < 0) {
    if (errno != EINTR) {
      return false;
    }
  }

  for (int i = 0; i < num_events; ++i) {
    FdChannel* pChannel = (FdChannel*)events[i].data.ptr;
    if (pChannel) {
      pChannel->handleEvent(events[i].events);
    }
  }
  return true;
#else
  return false;
#endif
}

NAMESPACE_END(net)
LY_NAMESPACE_END
