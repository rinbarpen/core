#include <chrono>
#include "core/net/SocketUtil.h"
#include "core/util/time/Timestamp.h"
#include <core/multimedia/net/rtsp/RtspPusher.h>
#include <core/net/tcp/TcpSocket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_rtsp_logger = GET_LOGGER("multimedia.rtsp");

auto RtspPusher::create(EventLoop *eventLoop) -> std::shared_ptr<RtspPusher>
{
  return std::make_shared<RtspPusher>(eventLoop);
}
RtspPusher::~RtspPusher(){this->close();}

void RtspPusher::addSession(MediaSession *session)
{
  Mutex::lock locker(mutex_);
  media_session_.reset(session);
}
void RtspPusher::removeSession(MediaSessionId session_id)
{
  Mutex::lock locker(mutex_);
	media_session_ = nullptr;
}

bool RtspPusher::openUrl(std::string_view url, std::chrono::milliseconds msec)
{
	std::lock_guard<std::mutex> lock(mutex_);

	static Timestamp<T_steady_clock> timestamp;
	timestamp.reset();

	if (!this->parseRtspUrl(url)) {
		ILOG_ERROR_FMT(g_rtsp_logger, "rtsp url({}) was illegal.\n", url);
		return false;
	}

	if (rtsp_conn_ != nullptr) {
		std::shared_ptr<RtspConnection> rtspConn = rtsp_conn_;
		sockfd_t sockfd = rtspConn->getSockfd();
		task_scheduler_->addTriggerEvent([sockfd, rtspConn] {
			rtspConn->disconnect();
		});
		rtsp_conn_ = nullptr;
	}

	auto tcp_socket = std::make_unique<TcpSocket>();
	if (!tcp_socket->connect({rtsp_url_info_.ip, rtsp_url_info_.port}, msec))
	{
		tcp_socket->close();
		return false;
	}

	task_scheduler_ = event_loop_->getTaskScheduler().get();
	rtsp_conn_.reset(new RtspConnection(shared_from_this(), task_scheduler_, tcp_socket.getSockfd()));
    event_loop_->addTriggerEvent([this]() {
		rtsp_conn_->sendOptions(RtspConnection::PUSHING);
    });

	auto duration = Clock<T_steady_clock>::now().duration<std::chrono::milliseconds>(timestamp);
  int64_t rest = msec.count() - duration.count();

	do
	{
		Timer::sleep(100ms);
		timeout -= 100;
	} while (!rtsp_conn_->isRecord() && timeout > 0);

	if (!rtsp_conn_->isRecord()) {
		std::shared_ptr<RtspConnection> rtspConn = rtsp_conn_;
		sockfd_t sockfd = rtspConn->getSockfd();
		task_scheduler_->addTriggerEvent([sockfd, rtspConn]() {
			rtspConn->disconnect();
		});
		rtsp_conn_ = nullptr;
		return false;
	}

	return true;
}
void RtspPusher::close(){
  Mutex::lock locker(mutex_);

	if (rtsp_conn_ != nullptr) {
		auto rtsp_conn = rtsp_conn_;
		sockfd_t sockfd = rtsp_conn->getSockfd();
		task_scheduler_->addTriggerEvent([sockfd, rtsp_conn]() {
			rtsp_conn->disconnect();
		});
		rtsp_conn_ = nullptr;
	}
}
bool RtspPusher::isConnected()
{
	Mutex::lock locker(mutex_);

	if (rtsp_conn_ != nullptr) {
		return (rtsp_conn_->isRunning());
	}
	return false;
}

bool RtspPusher::pushFrame(MediaChannelId channel_id, SimAVFrame frame)
{
	Mutex::lock locker(mutex_);
	if (!media_session_ || !rtsp_conn_) {
		return false;
	}

	return media_session_->handleFrame(channel_id, frame);
}

RtspPusher::RtspPusher(EventLoop *eventLoop)
  : event_loop_(eventLoop)
{

}
auto RtspPusher::lookMediaSession(MediaSessionId session_id) -> MediaSession::ptr
{
	return media_session_;
}

NAMESPACE_END(net)
LY_NAMESPACE_END
