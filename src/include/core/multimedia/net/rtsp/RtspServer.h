#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <core/net/tcp/TcpServer.h>
#include <core/multimedia/net/rtsp/rtsp.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class RtspConnection;
class RtspServer : public Rtsp, public TcpServer
{
public:
	static auto create(EventLoop* event_loop) -> std::shared_ptr<RtspServer>;

	RtspServer(EventLoop* event_loop);
	~RtspServer() = default;

  MediaSessionId addSession(MediaSession* session);
  void removeSession(MediaSessionId sessionId);

  bool pushFrame(MediaSessionId sessionId, MediaChannelId channel_id, SimAVFrame frame);

  auto type() const -> std::string override { return "RtspServer"; }
private:
  friend class RtspConnection;

  auto lookMediaSession(const std::string& suffix) -> MediaSession::ptr override;
  auto lookMediaSession(MediaSessionId session_id) -> MediaSession::ptr override;
  virtual auto onConnect(sockfd_t sockfd) -> TcpConnection::ptr override;

  std::unordered_map<MediaSessionId, std::shared_ptr<MediaSession>> media_sessions_;
  std::unordered_map<std::string, MediaSessionId> rtsp_suffix_map_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END

