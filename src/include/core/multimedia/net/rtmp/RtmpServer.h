#pragma once

#include <memory>
#include <core/util/marcos.h>
#include <core/net/tcp/TcpServer.h>
#include <core/multimedia/net/rtmp/rtmp.h>
#include <core/multimedia/net/rtmp/RtmpConnection.h>
#include <core/multimedia/net/rtmp/RtmpSession.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class RtmpSession;
class RtmpServer : public TcpServer, public Rtmp, public std::enable_shared_from_this<RtmpServer>
{
	friend class RtmpConnection;
	friend class HttpFlvConnection;
public:
	using EventCallback = std::function<void(std::string event_type, std::string stream_path)>;

	static std::shared_ptr<RtmpServer> create(EventLoop* event_loop);
	RtmpServer(EventLoop *event_loop);
  ~RtmpServer();

	void setEventCallback(EventCallback event_callback);

  virtual std::string type() const override { return "RtmpServer"; }

private:
	void addSession(std::string stream_path);
	void removeSession(std::string stream_path);

	std::shared_ptr<RtmpSession> getSession(std::string stream_path);
	bool hasSession(std::string stream_path) const;
	bool hasPublisher(std::string stream_path);

	void notifyEvent(std::string event_type, std::string stream_path);

  virtual TcpConnection::ptr onConnect(sockfd_t sockfd) override;

private:
  std::unordered_map<std::string, std::shared_ptr<RtmpSession>> rtmp_sessions_;
  std::vector<EventCallback> event_callbacks_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
