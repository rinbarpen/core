#pragma once
#include <core/net/EventLoop.h>
#include <core/net/FdChannel.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(net)

class SelectEventLoop : public EventLoop
{
public:
  SHARED_REG(SelectEventLoop);

  SelectEventLoop(EventLoopId id);
  ~SelectEventLoop();

  void updateChannel(FdChannel::ptr pChannel) override;
  void removeChannel(sockfd_t fd) override;

  bool handleEvent(std::chrono::milliseconds timeout = 0ms) override;

private:
  sockfd_t max_fd_;
  fd_set fd_read_backup_;
  fd_set fd_write_backup_;
  fd_set fd_exp_backup_;

  bool is_fd_read_reset_ = false;
  bool is_fd_write_reset_ = false;
  bool is_fd_exp_reset_ = false;

  Mutex::type mutex_;

  std::unordered_map<sockfd_t, FdChannel::ptr> channels_;
};

NAMESPACE_END(net)

LY_NAMESPACE_END
