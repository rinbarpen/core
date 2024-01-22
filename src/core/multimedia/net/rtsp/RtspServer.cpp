#include "core/util/container_util.h"
#include <core/util/Mutex.h>
#include "core/multimedia/net/MediaSession.h"
#include "core/multimedia/net/media.h"
#include "core/multimedia/net/rtp/RtpConnection.h"
#include <core/multimedia/net/rtsp/RtspServer.h>
#include <core/multimedia/net/rtsp/RtspConnection.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
auto create(EventLoop* event_loop) -> std::shared_ptr<RtspServer>
{
  return std::make_shared<RtspServer>(event_loop);
}
MediaSessionId RtspServer::addSession(MediaSession* session)
{
  Mutex::lock locker(mutex_);

  if (container_util::contain(rtsp_suffix_map_, session->getRtspUrlSuffix())) {
    return 0;
  }

  MediaSession::ptr media_session(session);
  MediaSessionId session_id = media_session->getMediaSessionId();
  rtsp_suffix_map_.emplace(session->getRtspUrlSuffix(), session_id);
  media_sessions_.emplace(session_id, media_session);

  return session_id;
}
void RtspServer::removeSession(MediaSessionId session_id)
{
  Mutex::lock locker(mutex_);

  auto value = container_util::get_if_contain(media_sessions_, session_id);
  if (value.has_value()) {
    rtsp_suffix_map_.erase(value.value()->getRtspUrlSuffix());
    media_sessions_.erase(session_id);
    return ;
  }
}

bool RtspServer::pushFrame(MediaSessionId session_id, MediaChannelId channel_id, SimAVFrame frame)
{
  std::shared_ptr<MediaSession> pSession;
  {
    std::lock_guard<std::mutex> locker(mutex_);
    auto value = container_util::get_if_contain(media_sessions_, session_id);
    if (!value.has_value()) {
      return false;
    }
    pSession = value.value();
  }

  if (pSession != nullptr && pSession->getClientCount() > 0) {
    return pSession->handleFrame(channel_id, frame);
  }

  return false;
}

RtspServer::RtspServer(EventLoop* event_loop)
  : TcpServer(event_loop)
{}
auto RtspServer::lookMediaSession(const std::string& suffix) -> MediaSession::ptr
{
  auto x = container_util::get_if_contain(rtsp_suffix_map_, suffix);

  return x.has_value() ? this->lookMediaSession(x.value()) : nullptr;
}
auto RtspServer::lookMediaSession(MediaSessionId session_id) -> MediaSession::ptr
{
  auto x = container_util::get_if_contain(media_sessions_, session_id);
  return x.has_value() ? x.value() : nullptr;
}
auto RtspServer::onConnect(sockfd_t sockfd) -> std::shared_ptr<TcpConnection>
{
  return std::make_shared<RtspConnection>(shared_from_this(), event_loop_->getTaskScheduler().get(), sockfd);
}
NAMESPACE_END(net)
LY_NAMESPACE_END
