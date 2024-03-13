#include <core/net/tcp/TcpServer.h>
#include <core/multimedia/net/http/HttpFlvConnection.h>
#include <core/multimedia/net/rtmp/RtmpServer.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class RtmpServer;

class HttpFlvServer : public TcpServer
{
public:
	explicit HttpFlvServer(EventLoop* event_loop);
	~HttpFlvServer();

	void attach(std::shared_ptr<RtmpServer> rtmp_server);

	std::string type() const override { return "HttpFlvServer"; }

private:
	TcpConnection::ptr onConnect(sockfd_t sockfd) override;

private:
	Mutex::type mutex_;
	std::weak_ptr<RtmpServer> rtmp_server_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
