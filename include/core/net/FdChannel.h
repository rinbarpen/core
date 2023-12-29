#pragma once
#include <functional>

#include <core/util/marcos.h>
#include <core/net/Socket.h>

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
  using EventCallbackFn = std::function<void()>;

  FdChannel(sockfd_t fd)
    :
    sockfd_(fd)
  {}
  virtual ~FdChannel() = default;

  virtual void handleEvent() { handleEvent(events_); }
  virtual void handleEvent(int events)
  {
    auto g_net_logger = GET_LOGGER("Net");
    ILOG_DEBUG(g_net_logger) << "FdChannel handle events: " << events;
    if (events & (EVENT_PRI | EVENT_IN)) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle READ event";
      read_cb_();
    }
    if (events & EVENT_OUT) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle WRITE event";
      write_cb_();
    }
    if (events & EVENT_HUP) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle CLOSE event";
      close_cb_();
      return;
    }
    if (events & EVENT_ERR) {
      ILOG_DEBUG(g_net_logger) << "FdChannel handle ERROR event";
      error_cb_();
    }
  }

  void setReadCallback(EventCallbackFn cb)
  {
    read_cb_ = cb;
  }
  void setWriteCallback(EventCallbackFn cb)
  {
    write_cb_ = cb;
  }
  void setCloseCallback(EventCallbackFn cb)
  {
    close_cb_ = cb;
  }
  void setErrorCallback(EventCallbackFn cb)
  {
    error_cb_ = cb;
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

  EventCallbackFn read_cb_;
  EventCallbackFn write_cb_;
  EventCallbackFn close_cb_;
  EventCallbackFn error_cb_;

  int events_{EVENT_NONE};
};

NAMESPACE_END(net)
LY_NAMESPACE_END
