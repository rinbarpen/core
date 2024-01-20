#pragma once

#include "core/multimedia/net/media.h"
#include "core/net/TaskScheduler.h"
#include "core/net/tcp/TcpConnection.h"
#include "core/util/ds/SharedString.h"
#include "core/util/marcos.h"
#include <atomic>
#include <core/multimedia/net/rtsp/rtsp.h>
#include <core/multimedia/net/rtsp/RtspMessage.h>
#include <core/util/Authentication.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class RtspServer;
class MediaSession;

class RtspConnection : public TcpConnection
{
  friend class RtpConnection;
  friend class MediaSession;
  friend class RtspServer;
  friend class RtspPusher;
public:
  using CloseCallback = std::function<void(sockfd_t)>;

  enum class ConnectionMode
  {
    RTSP_SERVER,
    RTSP_PUSHER,
    // RTSP_CLIENT,
  };

  enum class ConnectionStatus
  {
    CONNECTING,
    PLAYING,
    PUSHING,
  };

  // RtspConnection() = delete;
  RtspConnection(Rtsp::ptr rtsp_server, TaskScheduler *task_scheduler, sockfd_t sockfd);
  virtual ~RtspConnection();

  auto mediaSessionId() const -> MediaSessionId { return session_id_; }
  auto taskScheduler() const -> TaskScheduler* { return task_scheduler_; }

  void keepAlive() {
    // alive_count_.fetch_add(alive_count_.load() + 1, std::memory_order_relaxed);
    ++alive_count_;
  }
  auto isAlive() const -> bool;
  void resetAliveCount() { alive_count_ = 0; }

  auto getId() const -> TaskSchedulerId { return task_scheduler_->getId(); }

  auto isPlaying() const -> bool { return conn_status_ == ConnectionStatus::PLAYING; }
  auto isRecording() const -> bool { return conn_status_ == ConnectionStatus::PUSHING; }

private:
  auto onRead(BufferReader &buffer) -> bool;
	void onClose();
	void handleRtcp(sockfd_t sockfd);
	void handleRtcp(BufferReader& buffer);
	auto handleRtspRequest(BufferReader& buffer) -> bool;
	auto handleRtspResponse(BufferReader& buffer) -> bool;

	void sendRtspMessage(const char *buf, size_t len);

	void handleCmdOption();
	void handleCmdDescribe();
	void handleCmdSetup();
	void handleCmdPlay();
	void handleCmdTeardown();
	void handleCmdGetParameter();
	auto handleAuthentication() -> bool;

	void sendOptions(ConnectionMode mode = ConnectionMode::RTSP_SERVER);
	void sendDescribe();
	void sendAnnounce();
	void sendSetup();
	void handleRecord();

private:
	std::atomic_int alive_count_{1};
	std::weak_ptr<Rtsp> rtsp_;

	ConnectionMode  conn_mode_{ConnectionMode::RTSP_SERVER};
	ConnectionStatus conn_status_{ConnectionStatus::CONNECTING};
	MediaSessionId session_id_{0};

	bool has_auth_{true};
	std::string nonce_;
	std::unique_ptr<Authentication> auth_info_;

	FdChannel::ptr       rtp_channel_;
	FdChannel::ptr       rtcp_channels_[kMaxMediaChannel];
	std::unique_ptr<RtspRequest>   rtsp_request_;
	std::unique_ptr<RtspResponse>  rtsp_response_;
	std::shared_ptr<RtpConnection> rtp_conn_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
