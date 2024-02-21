#include <core/util/time/Timestamp.h>
#include <core/util/logger/Logger.h>
#include <core/net/tcp/TcpSocket.h>
#include <core/multimedia/net/rtmp/RtmpClient.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_rtmp_logger = GET_LOGGER("multimedia.rtmp");

RtmpClient::RtmpClient(EventLoop *event_loop)
	: event_loop_(event_loop)
{}

RtmpClient::~RtmpClient() {}

std::shared_ptr<RtmpClient> RtmpClient::create(EventLoop* event_loop)
{
	return std::make_shared<RtmpClient>(event_loop);
}

void RtmpClient::setFrameCallBack(FrameCallback callback)
{
	Mutex::lock locker(mutex_);
	frame_callback_ = callback;
}

bool RtmpClient::openUrl(std::string url, std::chrono::milliseconds msec, std::string& status)
{
	return this->openUrl(url, msec.count(), status);
}
bool RtmpClient::openUrl(std::string url, int msec, std::string& status)
{
	Mutex::lock locker(mutex_);

	static Timestamp timestamp;
	auto timeout = msec;
	if (timeout <= 0) {
		timeout = 5000;
	}

	timestamp.reset();
	if (this->parseRtmpUrl(url) != 0) {
		ILOG_INFO_FMT(g_rtmp_logger, "[RtmpPublisher] rtmp url({}) was illegal.", url);
		return false;
	}

	//LOG_INFO("[RtmpPublisher] ip:%s, port:%hu, stream path:%s\n", ip_.c_str(), port_, stream_path_.c_str());

	if (rtmp_conn_ != nullptr) {
		std::shared_ptr<RtmpConnection> rtmp_conn = rtmp_conn_;
		sockfd_t sockfd = rtmp_conn->getSockfd();
		task_scheduler_->addTriggerEvent([sockfd, rtmp_conn]() {
			rtmp_conn->disconnect();
		});
		rtmp_conn_ = nullptr;
	}

	auto tcp_socket = std::make_unique<TcpSocket>();
	if (!tcp_socket->connect({ip_, port_}, std::chrono::milliseconds(timeout))) {
		tcp_socket->close();
		return false;
	}

	task_scheduler_ = event_loop_->getTaskScheduler().get();
	rtmp_conn_.reset(new RtmpConnection(shared_from_this(), task_scheduler_, tcp_socket->getSockfd()));
	task_scheduler_->addTriggerEvent([this]() {
		if (frame_callback_) {
			rtmp_conn_->setPlayCallBack(frame_callback_);
		}
		rtmp_conn_->handshake();
	});

	timeout -= timestamp.elapsed().count();
	if (timeout < 0) {
		timeout = 1000;
	}

	do {
		Timer::sleep(100ms);
		timeout -= 100;
	} while (rtmp_conn_->isRunning() && !rtmp_conn_->isPlaying() && timeout > 0);

	status = rtmp_conn_->getStatus();
	if (!rtmp_conn_->isPlaying()) {
		std::shared_ptr<RtmpConnection> rtmp_conn = rtmp_conn_;
		sockfd_t sockfd = rtmp_conn->getSockfd();
		task_scheduler_->addTriggerEvent([sockfd, rtmp_conn]() {
			rtmp_conn->disconnect();
		});
		rtmp_conn_ = nullptr;
		return false;
	}

	return true;
}

void RtmpClient::close()
{
	Mutex::lock locker(mutex_);

	if (rtmp_conn_ != nullptr) {
		std::shared_ptr<RtmpConnection> rtmp_conn = rtmp_conn_;
		sockfd_t sockfd = rtmp_conn->getSockfd();
		task_scheduler_->addTriggerEvent([sockfd, rtmp_conn]() {
			rtmp_conn->disconnect();
		});
	}
}

bool RtmpClient::isConnected()
{
	Mutex::lock locker(mutex_);
  return rtmp_conn_ && rtmp_conn_->isRunning();
}

NAMESPACE_END(net)
LY_NAMESPACE_END
