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
	static std::shared_ptr<RtspServer> create(EventLoop* event_loop);

	RtspServer(EventLoop* event_loop);
	~RtspServer() = default;

  MediaSessionId addSession(MediaSession* session);
  void removeSession(MediaSessionId sessionId);

  bool pushFrame(MediaSessionId sessionId, MediaChannelId channel_id, SimAVFrame frame);

  std::string type() const override { return "RtspServer"; }
private:
  friend class RtspConnection;

  MediaSession::ptr lookMediaSession(const std::string& suffix) override;
  MediaSession::ptr lookMediaSession(MediaSessionId session_id) override;
  virtual TcpConnection::ptr onConnect(sockfd_t sockfd) override;

  std::unordered_map<MediaSessionId, std::shared_ptr<MediaSession>> media_sessions_;
  std::unordered_map<std::string, MediaSessionId> rtsp_suffix_map_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END

