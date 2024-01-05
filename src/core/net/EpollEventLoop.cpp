#include <core/net/EpollEventLoop.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_net_logger = GET_LOGGER("net");

EpollEventLoop::EpollEventLoop(EventLoopId id)
  :
  EventLoop(id)
{
#if defined(__LINUX__)
  epfd_ = ::epoll_create1(0);
#endif
}

EpollEventLoop::~EpollEventLoop()
{
#if defined(__LINUX__)
  if (epfd_ > 0) ::close(epfd_);
#endif
}

int EpollEventLoop::control(sockfd_t sockfd, int op, int events)
{
#if defined(__LINUX__)
  int err;
  struct epoll_event event;
  ::memset(&event, 0, sizeof(event));

  {
    Mutex::lock locker(mutex_);
    event.events = events;
    err = ::epoll_ctl(epfd_, op, sockfd, &event);
    if (err < 0) {
      ILOG_ERROR(g_net_logger) << "epoll_ctl error";
      return err;
    }

    if (op == EventCTL::DEL)
      this->removeChannel(sockfd);
    else
      channels_[sockfd]->setEvents(events);
  }

  return err;
#else
  return -1;
#endif
}
void EpollEventLoop::updateChannel(FdChannel::ptr pChannel)
{
  Mutex::lock locker(mutex_);
  channels_[pChannel->getSockfd()] = pChannel;
}
void EpollEventLoop::removeChannel(sockfd_t sockfd)
{
  Mutex::lock locker(mutex_);
  if (auto it = channels_.find(sockfd);
      it != channels_.end()) {
    channels_.erase(it);
  }
}
bool EpollEventLoop::handleEvent(std::chrono::milliseconds timeout)
{
#if defined(__LINUX__)
  const int MAX_EVENTS = 100;
  struct epoll_event events[MAX_EVENTS];

  Mutex::lock locker(mutex_);
  int num_events = ::epoll_wait(epfd_, events, MAX_EVENTS, timeout.count());
  if (num_events < 0) {
    if (errno != EINTR) {
      return false;
    }
  }

  for (int i = 0; i < num_events; ++i) {
    sockfd_t fd = events[i].data.fd;
    channels_[fd]->handleEvent(events[i].events);
  }
  return true;
#else
  return false;
#endif
}

NAMESPACE_END(net)
LY_NAMESPACE_END
