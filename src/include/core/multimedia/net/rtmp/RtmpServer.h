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
	static std::shared_ptr<RtmpServer> create(EventLoop* event_loop);
	RtmpServer(EventLoop *event_loop);
  ~RtmpServer();

  virtual std::string type() const override { return "RtmpServer"; }

  LY_NONCOPYABLE(RtmpServer);
private:
	void addSession(std::string stream_path);
	void removeSession(std::string stream_path);

	std::shared_ptr<RtmpSession> getSession(std::string stream_path);
	bool hasSession(std::string stream_path) const;
	bool hasPublisher(std::string stream_path);

  virtual TcpConnection::ptr onConnect(sockfd_t sockfd) override;

	EventLoop *event_loop_;
  std::unordered_map<std::string, std::shared_ptr<RtmpSession>> rtmp_sessions_;
  mutable Mutex::type mutex_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
