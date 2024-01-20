#include <memory>

#include "core/multimedia/net/media.h"
#include "core/net/SocketUtil.h"
#include "core/net/TaskScheduler.h"
#include "core/net/tcp/TcpConnection.h"
#include "core/util/Authentication.h"
#include "core/util/buffer/BufferReader.h"
#include "core/util/marcos.h"
#include <core/multimedia/net/rtsp/RtspConnection.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
RtspConnection::RtspConnection(Rtsp::ptr rtsp_server, TaskScheduler *task_scheduler, sockfd_t sockfd)
  : TcpConnection(task_scheduler, sockfd), rtsp_(rtsp_server), rtp_channel_(new FdChannel(sockfd)), rtsp_request_(new RtspRequest), rtsp_response_(new RtspResponse)
{
  this->setReadCallback([this](TcpConnection::ptr conn, BufferReader &reader){
    return this->onRead(reader);
  });
  this->setCloseCallback([this](TcpConnection::ptr conn){
    return this->onClose();
  });

  rtp_channel_->setReadCallback([this]{ TcpConnection::onRead(); });
  rtp_channel_->setWriteCallback([this]{ TcpConnection::onWrite(); });
  rtp_channel_->setCloseCallback([this]{ TcpConnection::onClose(); });
  rtp_channel_->setErrorCallback([this]{ TcpConnection::onError(); });

  for (int i = 0; i < kMaxMediaChannel; ++i) {
    rtcp_channels_[i] = nullptr;
  }

  if (rtsp_server->hasAuthInfo()) {
    auth_info_.reset(new Authentication(rtsp_server->auth()));
  }
}
RtspConnection::~RtspConnection() {}

auto RtspConnection::onRead(BufferReader &reader) -> bool
{
  keepAlive();
  if (reader.empty()) return false;

  if (conn_mode_ == ConnectionMode::RTSP_SERVER) {
    if (!handleRtspRequest(reader)) {
      return false;
    }
  }
  else if (conn_mode_ == ConnectionMode::RTSP_PUSHER) {
    if (!handleRtspResponse(reader)) {
      return false;
    }
  }

  if (reader.readableBytes() > 2048) {
    reader.readAll();
  }
  return true;
}

void RtspConnection::onClose()
{
	if(session_id_ != 0) {
		auto rtsp = rtsp_.lock();
		if (rtsp) {
			MediaSession::ptr media_session = rtsp->lookMediaSession(session_id_);
			if (media_session) {
				media_session->removeClient(this->getSockfd());
			}
		}
	}

	for(int chn=0; chn<kMaxMediaChannel; chn++) {
		if(rtcp_channels_[chn] && !rtcp_channels_[chn]->isNoneEvent()) {
			task_scheduler_->removeChannel(rtcp_channels_[chn]->getSockfd());
		}
	}
}

bool RtspConnection::handleRtspRequest(BufferReader& buffer)
{
#if 0
	std::string str(buffer.peek(), buffer.readableBytes());
	if (str.find("rtsp") != std::string::npos || str.find("RTSP") != std::string::npos)
	{
		std::cout << str << std::endl;
	}
#endif

	if (!rtsp_request_->parse(buffer)) {
    return false;
  }

  RtspMethod method = rtsp_request_->method();
  if(method.method == RtspMethod::RTCP) {
    this->handleRtcp(buffer);
    return true;
  }
  else if (!rtsp_request_->isGotAll()) {
    return true;
  }

  switch (method.method)
  {
  case RtspMethod::OPTIONS:
    this->handleCmdOption();
    break;
  case RtspMethod::DESCRIBE:
    this->handleCmdDescribe();
    break;
  case RtspMethod::SETUP:
    this->handleCmdSetup();
    break;
  case RtspMethod::PLAY:
    this->handleCmdPlay();
    break;
  case RtspMethod::TEARDOWN:
    this->handleCmdTeardown();
    break;
  case RtspMethod::GET_PARAMETER:
    this->handleCmdGetParameter();
    break;
  default:
    break;
  }

  if (rtsp_request_->isGotAll()) {
    rtsp_request_->reset();
  }

	return true;
}

bool RtspConnection::handleRtspResponse(BufferReader& buffer)
{
#if 0
	std::string str(buffer.Peek(), buffer.ReadableBytes());
	if (str.find("rtsp") != std::string::npos || str.find("RTSP") != std::string::npos) {
		std::cout << str << std::endl;
	}
#endif

	if (rtsp_response_->parse(buffer)) {
    return false;
  }

  RtspMethod method = rtsp_response_->method();
  switch (method.method)
  {
  case RtspMethod::OPTIONS:
    if (conn_mode_ == ConnectionMode::RTSP_PUSHER) {
      this->sendAnnounce();
    }
    break;
  case RtspMethod::ANNOUNCE:
  case RtspMethod::DESCRIBE:
    this->sendSetup();
    break;
  case RtspMethod::SETUP:
    this->sendSetup();
    break;
  case RtspMethod::RECORD:
    this->handleRecord();
    break;
  default:
    break;
  }

	return true;
}

void RtspConnection::sendRtspMessage(const char *buf, size_t len)
{
#if 0
	std::cout << buf << std::endl;
#endif

	this->send(buf, len);
	return;
}

void RtspConnection::handleRtcp(BufferReader& buffer)
{
	char *peek = buffer.peek();
	if(peek[0] == '$' && buffer.readableBytes() > 4) {
		uint32_t pkt_size = peek[2] << 8 | peek[3];
		if(pkt_size + 4 >=  buffer.readableBytes()) {
			buffer.advance(pkt_size + 4);
		}
	}
}

void RtspConnection::handleRtcp(sockfd_t sockfd)
{
	char buf[1024] = {0};
	if(socket_api::recv(sockfd, buf, 1024, 0) > 0) {
		this->keepAlive();
	}
}

void RtspConnection::handleCmdOption()
{
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());
	int size = rtsp_request_->buildOptionRes(res.get(), 2048);
	this->sendRtspMessage(res.get(), size);
}

void RtspConnection::handleCmdDescribe()
{
	if (auth_info_!=nullptr && !handleAuthentication()) {
		return;
	}

	if (rtp_conn_ == nullptr) {
		rtp_conn_.reset(new RtpConnection(shared_from_this()));
	}

	int size = 0;
	std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
	MediaSession::ptr media_session = nullptr;

	auto rtsp = rtsp_.lock();
	if (rtsp) {
		media_session = rtsp->lookMediaSession(rtsp_request_->rtspUrlSuffix());
	}

	if(!rtsp || !media_session) {
		size = rtsp_request_->buildNotFoundRes(res.get(), 4096);
	}
	else {
		session_id_ = media_session->getMediaSessionId();
		media_session->addClient(this->getSockfd(), rtp_conn_);

		for(int chn=0; chn<kMaxMediaChannel; chn++) {
			MediaSource* source = media_session->getMediaSource((MediaChannelId)chn);
			if (source != nullptr) {
				rtp_conn_->setClockRate((MediaChannelId)chn, source->getClockRate());
				rtp_conn_->setPayloadType((MediaChannelId)chn, source->getPayloadType());
			}
		}

		std::string sdp = media_session->getSdpMessage(socket_api::getSocketAddr(this->getSockfd()).ip, rtsp->version());
		if(sdp.empty()) {
			size = rtsp_request_->buildServerErrorRes(res.get(), 4096);
		}
		else {
			size = rtsp_request_->buildDescribeRes(res.get(), 4096, sdp.c_str());
		}
	}

	this->sendRtspMessage(res.get(), size);
}

void RtspConnection::handleCmdSetup()
{
	if (auth_info_ != nullptr && !this->handleAuthentication()) {
		return;
	}

	int size = 0;
	std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
	MediaChannelId channel_id = rtsp_request_->channelId();
	MediaSession::ptr media_session = nullptr;

	auto rtsp = rtsp_.lock();
	if (rtsp) {
		media_session = rtsp->lookMediaSession(session_id_);
	}

	if(!rtsp || !media_session)  {
		goto server_error;
	}

	if(media_session->isMulticasting())  {
		std::string multicast_ip = media_session->getMulticastIp();
		if (rtsp_request_->transportMode() == RTP_OVER_MULTICAST) {
			uint16_t port = media_session->getMulticastPort(channel_id);
			uint16_t session_id = rtp_conn_->getRtpSessionId();
			if (!rtp_conn_->setupRtpOverMulticast(channel_id, {multicast_ip, port})) {
				goto server_error;
			}

			size = rtsp_request_->buildSetupMulticastRes(res.get(), 4096, multicast_ip.c_str(), port, session_id);
		}
		else {
			goto transport_unsupported;
		}
	}
	else {
		if(rtsp_request_->transportMode() == RTP_OVER_TCP) {
			uint16_t rtp_channel = rtsp_request_->rtpChannel();
			uint16_t rtcp_channel = rtsp_request_->rtcpChannel();
			uint16_t session_id = rtp_conn_->getRtpSessionId();

			rtp_conn_->setupRtpOverTcp(channel_id, rtp_channel, rtcp_channel);
			size = rtsp_request_->buildSetupTcpRes(res.get(), 4096, rtp_channel, rtcp_channel, session_id);
		}
		else if(rtsp_request_->transportMode() == RTP_OVER_UDP) {
			uint16_t peer_rtp_port = rtsp_request_->rtpPort();
			uint16_t peer_rtcp_port = rtsp_request_->rtcpPort();
			uint16_t session_id = rtp_conn_->getRtpSessionId();

			if (rtp_conn_->setupRtpOverUdp(channel_id, peer_rtp_port, peer_rtcp_port)) {
				sockfd_t rtcp_fd = rtp_conn_->getRtcpSockfd(channel_id);
				rtcp_channels_[channel_id].reset(new FdChannel(rtcp_fd));
				rtcp_channels_[channel_id]->setReadCallback([rtcp_fd, this]() { this->handleRtcp(rtcp_fd); });
				rtcp_channels_[channel_id]->enableReading();
				task_scheduler_->updateChannel(rtcp_channels_[channel_id]);
			}
			else {
				goto server_error;
			}

			uint16_t serRtpPort = rtp_conn_->getRtpPort(channel_id);
			uint16_t serRtcpPort = rtp_conn_->getRtcpPort(channel_id);
			size = rtsp_request_->buildSetupUdpRes(res.get(), 4096, serRtpPort, serRtcpPort, session_id);
		}
		else {
			goto transport_unsupported;
		}
	}

	this->sendRtspMessage(res.get(), size);
	return ;

transport_unsupported:
	size = rtsp_request_->buildUnsupportedRes(res.get(), 4096);
	this->sendRtspMessage(res.get(), size);
	return ;

server_error:
	size = rtsp_request_->buildServerErrorRes(res.get(), 4096);
	this->sendRtspMessage(res.get(), size);
	return ;
}

void RtspConnection::handleCmdPlay()
{
	if (auth_info_ != nullptr) {
		if (!handleAuthentication()) {
			return;
		}
	}

	if (rtp_conn_ == nullptr) {
		return;
	}

	conn_status_ = ConnectionStatus::PLAYING;
	rtp_conn_->play();

	uint16_t session_id = rtp_conn_->getRtpSessionId();
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());

	int size = rtsp_request_->buildPlayRes(res.get(), 2048, nullptr, session_id);
	this->sendRtspMessage(res.get(), size);
}

void RtspConnection::handleCmdTeardown()
{
	if (rtp_conn_ == nullptr) {
		return;
	}
  rtp_conn_->teardown();

  uint16_t session_id = rtp_conn_->getRtpSessionId();

	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());
	int size = rtsp_request_->buildTeardownRes(res.get(), 2048, session_id);
	this->sendRtspMessage(res.get(), size);

	//HandleClose();
}

void RtspConnection::handleCmdGetParameter()
{
	if (rtp_conn_ == nullptr) {
		return;
	}

	uint16_t session_id = rtp_conn_->getRtpSessionId();
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());
	int size = rtsp_request_->buildGetParameterRes(res.get(), 2048, session_id);
	this->sendRtspMessage(res.get(), size);
}

bool RtspConnection::handleAuthentication()
{
	if (auth_info_ != nullptr && !has_auth_) {
		std::string cmd = rtsp_request_->method().toString();
		std::string url = rtsp_request_->rtspUrl();

		if (!nonce_.empty() && (auth_info_->getResponse(nonce_, cmd, url) == rtsp_request_->authResponse())) {
			nonce_.clear();
			has_auth_ = true;
		}
		else {
			std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
			nonce_ = auth_info_->getNonce();
			int size = rtsp_request_->buildUnauthorizedRes(res.get(), 4096, auth_info_->getRealm().c_str(), nonce_.c_str());
			this->sendRtspMessage(res.get(), size);
			return false;
		}
	}

	return true;
}

void RtspConnection::sendOptions(ConnectionMode mode)
{
	if (rtp_conn_ == nullptr) {
		rtp_conn_.reset(new RtpConnection(shared_from_this()));
	}

	auto rtsp = rtsp_.lock();
	if (!rtsp) {
		TcpConnection::onClose();
		return;
	}

	conn_mode_ = mode;
	rtsp_response_->setUserAgent("-_-");
	rtsp_response_->setRtspUrl(rtsp->rtspUrl().c_str());

	std::shared_ptr<char> req(new char[2048], std::default_delete<char[]>());
	int size = rtsp_response_->buildOptionReq(req.get(), 2048);
	this->sendRtspMessage(req.get(), size);
}

void RtspConnection::sendAnnounce()
{
	MediaSession::ptr media_session = nullptr;

	auto rtsp = rtsp_.lock();
	if (rtsp) {
		media_session = rtsp->lookMediaSession(1);
	}

	if (!rtsp || !media_session) {
		TcpConnection::onClose();
		return;
	}
	else {
		session_id_ = media_session->getMediaSessionId();
		media_session->addClient(this->getSockfd(), rtp_conn_);

		for (int chn = 0; chn<2; chn++) {
			MediaSource* source = media_session->getMediaSource((MediaChannelId)chn);
			if (source != nullptr) {
				rtp_conn_->setClockRate((MediaChannelId)chn, source->getClockRate());
				rtp_conn_->setPayloadType((MediaChannelId)chn, source->getPayloadType());
			}
		}
	}

	std::string sdp = media_session->getSdpMessage(socket_api::getSocketAddr(this->getSockfd()).ip, rtsp->version());
	if (sdp.empty()) {
		TcpConnection::onClose();
		return;
	}

	std::shared_ptr<char> req(new char[4096], std::default_delete<char[]>());
	int size = rtsp_response_->buildAnnounceReq(req.get(), 4096, sdp.c_str());
	this->sendRtspMessage(req.get(), size);
}

void RtspConnection::sendDescribe()
{
	std::shared_ptr<char> req(new char[2048], std::default_delete<char[]>());
	int size = rtsp_response_->buildDescribeReq(req.get(), 2048);
	this->sendRtspMessage(req.get(), size);
}

void RtspConnection::sendSetup()
{
	int size = 0;
	MediaSession::ptr media_session = nullptr;
	std::shared_ptr<char> buf(new char[2048], std::default_delete<char[]>());

	auto rtsp = rtsp_.lock();
	if (rtsp) {
		media_session = rtsp->lookMediaSession(session_id_);
	}
	if (!rtsp || !media_session) {
		TcpConnection::onClose();
		return;
	}

	if (media_session->getMediaSource(channel_0) && !rtp_conn_->isSetup(channel_0)) {
		rtp_conn_->setupRtpOverTcp(channel_0, 0, 1);
		size = rtsp_response_->buildSetupTcpReq(buf.get(), 2048, channel_0);
	}
	else if (media_session->getMediaSource(channel_1) && !rtp_conn_->isSetup(channel_1)) {
		rtp_conn_->setupRtpOverTcp(channel_1, 2, 3);
		size = rtsp_response_->buildSetupTcpReq(buf.get(), 2048, channel_1);
	}
	else {
		size = rtsp_response_->buildRecordReq(buf.get(), 2048);
	}

	this->sendRtspMessage(buf.get(), size);
}

void RtspConnection::handleRecord()
{
	conn_status_ = ConnectionStatus::PUSHING;
	rtp_conn_->record();
}

NAMESPACE_END(net)
LY_NAMESPACE_END
