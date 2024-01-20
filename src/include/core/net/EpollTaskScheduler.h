#pragma once
#include <core/util/marcos.h>
#include <core/net/FdChannel.h>
#include <core/net/TaskScheduler.h>

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(net)

class EpollTaskScheduler final : public TaskScheduler
{
public:
  SHARED_REG(EpollTaskScheduler);
#if defined(__LINUX__)
  enum EventCTL {
    ADD = EPOLL_CTL_ADD,
    MOD = EPOLL_CTL_MOD,
    DEL = EPOLL_CTL_DEL,
  };
  enum EventType {
    READABLE = EPOLLIN,
    WRITABLE = EPOLLOUT,
    URGENT_READ = EPOLLPRI,
    ERR = EPOLLERR,
    HUP = EPOLLHUP,
    ETMODE = EPOLLET /* EPOLLLT */,
    ONE_SHOT = EPOLLONESHOT,
  };
#else
  enum EventCTL {
    // ADD = EPOLL_CTL_ADD,
    // MOD = EPOLL_CTL_MOD,
    // DEL = EPOLL_CTL_DEL,
  };
  enum EventType {
    // READABLE = EPOLLIN,
    // WRITABLE = EPOLLOUT,
    // URGENT_READ = EPOLLPRI,
    // ERR = EPOLLERR,
    // HUP = EPOLLHUP,
    // ETMODE = EPOLLET /* EPOLLLT */,
    // ONE_SHOT = EPOLLONESHOT,
  };
#endif
  EpollTaskScheduler(TaskSchedulerId id);
  ~EpollTaskScheduler();


  void updateChannel(FdChannel::ptr pChannel) override;
  void removeChannel(sockfd_t sockfd) override;

  bool handleEvent(std::chrono::milliseconds timeout = 0ms) override;

private:
  void control(int op, FdChannel::ptr pChannel);

private:
  int epfd_;

  Mutex::type mutex_;

  std::unordered_map<sockfd_t, FdChannel::ptr> channels_;
};

NAMESPACE_END(net)

LY_NAMESPACE_END
