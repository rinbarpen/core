#pragma once

#include <chrono>
#include <memory>

#include <core/net/EventLoop.h>
#include <core/multimedia/net/media.h>
#include <core/multimedia/net/MediaSession.h>
#include <core/multimedia/net/rtp/RtpConnection.h>
#include <core/multimedia/net/rtsp/rtsp.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class RtspConnection;
class RtspPusher : public Rtsp
{
  friend class RtspConnection;
public:
  static auto create(EventLoop *eventLoop) -> std::shared_ptr<RtspPusher>;
  RtspPusher(EventLoop *eventLoop);
  ~RtspPusher();

  void addSession(MediaSession *session);
  void removeSession(MediaSessionId session_id);

  bool openUrl(std::string_view url, std::chrono::milliseconds msec = 3000ms);
  void close();
  bool isConnected();

  bool pushFrame(MediaChannelId channel_id, SimAVFrame frame);

  LY_NONCOPYABLE(RtspPusher);
private:
  auto lookMediaSession(MediaSessionId session_id) -> MediaSession::ptr override;

private:
  EventLoop *event_loop_{nullptr};
  TaskScheduler *task_scheduler_{nullptr};
  Mutex::type mutex_;

  std::shared_ptr<RtspConnection> rtsp_conn_;
  std::shared_ptr<MediaSession> media_session_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
