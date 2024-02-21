#include <core/multimedia/net/rtmp/RtmpServer.h>
#include <memory>
#include "core/multimedia/net/rtmp/RtmpSession.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
RtmpServer::RtmpServer(EventLoop* event_loop)
	: TcpServer(event_loop)
	, event_loop_(event_loop)
{
	event_loop_->addTimer({[this] {
		std::lock_guard<std::mutex> lock(mutex_);
		for (auto iter = rtmp_sessions_.begin(); iter != rtmp_sessions_.end(); ) {
			if (iter->second->getClients() == 0) {
				rtmp_sessions_.erase(iter++);
			}
			else {
				iter++;
			}
		}
		return true;
	}, 30000ms});
}
RtmpServer::~RtmpServer() {}

std::shared_ptr<RtmpServer> RtmpServer::create(EventLoop* event_loop)
{
  return std::make_shared<RtmpServer>(event_loop);
}

void RtmpServer::addSession(std::string stream_path) {
	Mutex::lock locker(mutex_);
	// if (rtmp_sessions_.find(stream_path) == rtmp_sessions_.end())
	// 	rtmp_sessions_[stream_path] = std::make_shared<RtmpSession>();
	rtmp_sessions_.emplace(stream_path, std::make_shared<RtmpSession>());
}
void RtmpServer::removeSession(std::string stream_path) {
	Mutex::lock locker(mutex_);
	rtmp_sessions_.erase(stream_path);
}

std::shared_ptr<RtmpSession> RtmpServer::getSession(std::string stream_path) {
	Mutex::lock locker(mutex_);
	if (rtmp_sessions_.find(stream_path) == rtmp_sessions_.end()) {
		rtmp_sessions_[stream_path] = std::make_shared<RtmpSession>();
	}

	return rtmp_sessions_[stream_path];
}
bool RtmpServer::hasSession(std::string stream_path) const {
	Mutex::lock locker(mutex_);
	return (rtmp_sessions_.find(stream_path) != rtmp_sessions_.end());
}
bool RtmpServer::hasPublisher(std::string stream_path) {
	auto session = this->getSession(stream_path);
	if (session == nullptr) {
		return false;
	}

	return (session->getPublisher() != nullptr);
}

TcpConnection::ptr RtmpServer::onConnect(sockfd_t sockfd)
{
  return std::make_shared<RtmpConnection>(shared_from_this(), event_loop_->getTaskScheduler().get(), sockfd);
}

NAMESPACE_END(net)
LY_NAMESPACE_END
