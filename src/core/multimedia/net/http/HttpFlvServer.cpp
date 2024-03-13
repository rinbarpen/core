#include <core/multimedia/net/http/HttpFlvServer.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_http_flv_logger = GET_LOGGER("multimedia.http");

HttpFlvServer::HttpFlvServer(EventLoop *event_loop)
  : TcpServer(event_loop)
{}
HttpFlvServer::~HttpFlvServer() {}

void HttpFlvServer::attach(std::shared_ptr<RtmpServer> rtmp_server) {
  Mutex::lock locker(mutex_);
  rtmp_server_ = rtmp_server;
}

TcpConnection::ptr HttpFlvServer::onConnect(sockfd_t sockfd) {
  auto rtmp_server = rtmp_server_.lock();
  if (rtmp_server) {
    ILOG_DEBUG(g_http_flv_logger) << "A new connection comes";
    return std::make_shared<HttpFlvConnection>(rtmp_server, event_loop_->getTaskScheduler().get(), sockfd);
  }
  return nullptr;
}

NAMESPACE_END(net)
LY_NAMESPACE_END
