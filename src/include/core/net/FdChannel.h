#pragma once
#include <functional>

#include <core/util/marcos.h>
#include <core/net/Socket.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

enum EventType
{
  EVENT_NONE = 0,
  EVENT_IN  = 1,
  EVENT_PRI = 2,
  EVENT_OUT = 4,
  EVENT_ERR = 8,
  EVENT_HUP = 16,
  EVENT_RDHUP = 8192
};

/**
 * \brief handle {read, write, error, close} event
 *
 */
class FdChannel
{
public:
  SHARED_REG(FdChannel);
  using EventCallback = std::function<void()>;

  FdChannel(sockfd_t fd)
    : sockfd_(fd)
  {}
  virtual ~FdChannel() = default;

  virtual void handleEvent(int events)
  {
    auto g_net_logger = GET_LOGGER("net");
    ILOG_DEBUG(g_net_logger) << "FdChannel handle events: " << events;
    if (events & (EVENT_PRI | EVENT_IN)) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle READ event";
      read_callback_();
    }
    if (events & EVENT_OUT) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle WRITE event";
      write_callback_();
    }
    if (events & EVENT_HUP) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle CLOSE event";
      close_callback_();
      return;
    }
    if (events & EVENT_ERR) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle ERROR event";
      error_callback_();
    }
  }

  void setReadCallback(EventCallback callback)
  {
    read_callback_ = callback;
  }
  void setWriteCallback(EventCallback callback)
  {
    write_callback_ = callback;
  }
  void setCloseCallback(EventCallback callback)
  {
    close_callback_ = callback;
  }
  void setErrorCallback(EventCallback callback)
  {
    error_callback_ = callback;
  }

  void enableReading()
  {
    events_ |= EVENT_IN;
  }
  void enableWriting()
  {
    events_ |= EVENT_OUT;
  }
  void disableReading()
  {
    events_ &= ~EVENT_IN;
  }
  void disableWriting()
  {
    events_ &= ~EVENT_OUT;
  }

  int  getEvents() const { return events_; }
  void setEvents(int events) { events_ = events; }

  bool isNoneEvent() const { return events_ == EVENT_NONE; }
  bool isReading() const { return (events_ & EVENT_IN) != 0; }
  bool isWriting() const { return (events_ & EVENT_OUT) != 0; }

  sockfd_t getSockfd() const { return sockfd_; }
private:
  sockfd_t sockfd_;

  EventCallback read_callback_= []{};
  EventCallback write_callback_= []{};
  EventCallback close_callback_= []{};
  EventCallback error_callback_= []{};

  int events_{EVENT_NONE};
};

NAMESPACE_END(net)
LY_NAMESPACE_END
