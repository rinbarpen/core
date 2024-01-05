#include "MediaSession.h"

#include <forward_list>

#include "net/NetAddress.h"

MediaSession* MediaSession::create(std::string suffix)
{
	return new MediaSession(suffix);
}
MediaSession::~MediaSession()
{
	if (multicast_ip_ != "") {
	  SingleMulticastAddress::instance()->release(multicast_ip_);
	}
}
bool MediaSession::addSource(MediaChannelId channel_id, MediaSource* source)
{
	source->setSendFrameCallback([this](MediaChannelId channel_id, RtpPacket pkt) {
		std::forward_list<std::shared_ptr<RtpConnection>> clients;
		// TODO: Fixme! Add RtpConection::getId()
	  std::map<std::string, RtpPacket> packets;
	  {
		  Mutex::lock locker(map_mutex_);
		  for (auto iter = clients_.begin(); iter != clients_.end();) {
			  auto conn = iter->second.lock();
			  if (conn == nullptr) {
				  clients_.erase(iter++);
					continue;
			  }

				auto id = conn->getId();
				if (id != "") {
					if (packets.find(id) == packets.end()) {
						RtpPacket tmp_pkt;
						tmp_pkt.data.append(pkt.data.get(), pkt.data.size());
						tmp_pkt.last = pkt.last;
						tmp_pkt.timestamp = pkt.timestamp;
						tmp_pkt.type = pkt.type;
						packets.emplace(id, tmp_pkt);
					}
					clients.emplace_front(conn);
				}
				iter++;
		  }
	  }

	  int count = 0;
	  for (auto iter : clients) {
		  int ret = 0;
		  auto id = iter->getId();
		  if (id != "") {
			  auto iter2 = packets.find(id);
			  if (iter2 != packets.end()) {
				  count++;
				  ret = iter->sendRtpPacket(channel_id, iter2->second);
				  if (multicasting_ && ret == 0) {
					  break;
				  }
			  }
		  }
	  }
	  return true;
	});

	media_sources_[channel_id].reset(source);
	return true;
}
bool MediaSession::removeSource(MediaChannelId channel_id)
{
	media_sources_[channel_id] = nullptr;
	return true;
}
bool MediaSession::startMulticast()
{
	if (multicasting_) {
		return true;
	}

	multicast_ip_ = SingleMulticastAddress::instance()->getAddr();
	if (multicast_ip_ == "") {
		return false;
	}

	std::random_device rd;
	multicast_port_[channel_0] = htons(rd() & 0xfffe);
	multicast_port_[channel_1] = htons(rd() & 0xfffe);

	multicasting_ = true;
	return true;
}

void MediaSession::addNotifyConnectedCallback(const NotifyConnectedCallback& fn)
{
  notify_connected_cbs_.push_back(fn);
}
void MediaSession::addNotifyDisconnectedCallback(const NotifyDisconnectedCallback& fn)
{
  notify_disconnected_cbs_.push_back(fn);
}
std::string MediaSession::getSdpMessage(const std::string& ip, std::string session_name)
{
	if (sdp_ != "") {
		return sdp_;
	}

	if (media_sources_.empty()) {
		return "";
	}

	char buf[2048] = { 0 };

	snprintf(buf, sizeof(buf),
					 "v=0\r\n"
					 "o=- 9%ld 1 IN IP4 %s\r\n"
					 "t=0 0\r\n"
					 "a=control:*\r\n",
					 (long)std::time(NULL), ip.c_str());

	if (session_name != "") {
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
						 "s=%s\r\n",
						 session_name.c_str());
	}

	if (multicasting_) {
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
						 "a=type:broadcast\r\n"
						 "a=rtcp-unicast: reflection\r\n");
	}

	for (uint32_t chn = 0; chn < media_sources_.size(); chn++) {
		if (media_sources_[chn]) {
			if (multicasting_) {
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
								 "%s\r\n",
								 media_sources_[chn]->getMediaDescription(multicast_port_[chn]).c_str());

				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
								 "c=IN IP4 %s/255\r\n",
								 multicast_ip_.c_str());
			}
			else {
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
								 "%s\r\n",
								 media_sources_[chn]->getMediaDescription(0).c_str());
			}

			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
							 "%s\r\n",
							 media_sources_[chn]->getAttribute().c_str());

			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
							 "a=control:track%d\r\n", chn);
		}
	}

	sdp_ = buf;
	return sdp_;
}

MediaSource* MediaSession::getMediaSource(MediaSessionId channel_id)
{
  if (media_sources_[channel_id]) {
    return media_sources_[channel_id].get();
  }

  return nullptr;
}

bool MediaSession::handleFrame(MediaChannelId channel_id, SimAVFrame frame)
{
  Mutex::lock locker(mutex_);

  if (media_sources_[channel_id]) {
    media_sources_[channel_id]->handleFrame(channel_id, frame);
  }
  else {
    return false;
  }

  return true;
}

bool MediaSession::addClient(sockfd_t rtspfd, std::shared_ptr<RtpConnection> rtpConn)
{
  Mutex::lock locker(mutex_);
  if (auto it = clients_.begin(); 
      it != clients_.end()) {
    return false;
  }

  clients_.emplace(rtspfd, rtpConn);
  for (auto &cb : notify_connected_cbs_) {
    cb(session_id_, rtpConn->getRtspIp(), rtpConn->getRtspPort());
  }
  has_new_client_ = true;
  return true;
}

bool MediaSession::removeClient(sockfd_t rtspfd)
{
  Mutex::lock locker(mutex_);
  if (auto it = clients_.begin();
      it != clients_.end()) {
    auto rtpConn = clients_.at(rtspfd).lock();
    if (rtpConn) {
      for (auto &cb : notify_disconnected_cbs_) {
        cb(session_id_, rtpConn->getRtspIp(), rtpConn->getRtspPort());
      }
    }
    clients_.erase(it);

    return true;
  }

  return false;
}

uint16_t MediaSession::getMulticastPort(MediaChannelId channel_id) const
{
  if (channel_id >= kMaxMediaChannel) {
    return 0;
  }
  return multicast_port_[channel_id];
}

MediaSession::MediaSession(std::string url_suffix) :
  suffix_(url_suffix),
  media_sources_(kMaxMediaChannel),
	buffers_(kMaxMediaChannel)
{
	has_new_client_ = false;
	session_id_ = ++next_session_id_;

	for (int n = 0; n < kMaxMediaChannel; n++) {
		multicast_port_[n] = 0;
	}
}
