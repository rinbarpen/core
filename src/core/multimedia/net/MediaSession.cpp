#include <core/util/marcos.h>
#include <core/net/multicast/MulticastAddress.h>
#include <core/multimedia/net/media.h>
#include <core/multimedia/net/MediaSession.h>

#include <forward_list>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

MediaSession* MediaSession::create(std::string suffix)
{
	return new MediaSession(suffix);
}
MediaSession::~MediaSession()
{
	multicast_address_.release();
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
				if (!id.empty()) {
					if (packets.find(id) == packets.end()) {
						RtpPacket tmp_pkt;
						std::memcpy(tmp_pkt.data.get(), pkt.data.get(), pkt.size);
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
			  if (auto iter2 = packets.find(id);
						iter2 != packets.end()) {
				  count++;
				  ret = iter->sendRtpPacket(channel_id, iter2->second);
				  if (multicast_on_ && ret == 0) {
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
	if (multicast_on_) {
		return true;
	}
	if (multicast_address_.ip().empty()) {
		return false;
	}

	multicast_address_.clear();
	std::random_device rd;
	multicast_address_.addPort(htons(rd() & 0xFFFE));
	multicast_address_.addPort(htons(rd() & 0xFFFE));

	multicast_on_ = true;
	return true;
}

void MediaSession::addNotifyConnectedCallback(const NotifyConnectedCallback& callback)
{
  notify_connected_callbacks_.push_back(callback);
}
void MediaSession::addNotifyDisconnectedCallback(const NotifyDisconnectedCallback& callback)
{
  notify_disconnected_callbacks_.push_back(callback);
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

	if (multicast_on_) {
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
						 "a=type:broadcast\r\n"
						 "a=rtcp-unicast: reflection\r\n");
	}

	for (uint32_t chn = 0; chn < media_sources_.size(); chn++) {
		if (media_sources_[chn]) {
			if (multicast_on_) {
				//buf += fmt::vformat("{:s}\r\n", media_sources_[chn]->getMediaDescription(
				//					multicast_address_.port(chn)));
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
								 "%s\r\n",
								 media_sources_[chn]->getMediaDescription(
									multicast_address_.port(chn)).c_str());

				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
								 "c=IN IP4 %s/255\r\n",
								 multicast_address_.ip().c_str());
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

  if (!media_sources_[channel_id]) {
    return false;
  }

	media_sources_[channel_id]->handleFrame(channel_id, frame);
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
  for (auto &callback : notify_connected_callbacks_) {
    callback(session_id_, rtpConn->getRtspAddress().ip, rtpConn->getRtspAddress().port);
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
      for (auto &callback : notify_disconnected_callbacks_) {
        callback(session_id_, rtpConn->getRtspAddress().ip, rtpConn->getRtspAddress().port);
      }
    }
    clients_.erase(it);

    return true;
  }

  return false;
}

uint16_t MediaSession::getMulticastPort(MediaChannelId channel_id) const
{
	return multicast_address_.port(channel_id);
}

MediaSession::MediaSession(std::string url_suffix) :
  suffix_(url_suffix),
  media_sources_(kMaxMediaChannel),
	buffers_(kMaxMediaChannel),
	multicast_address_("", kMaxMediaChannel),
	session_id_(generator_.next())
{
}
NAMESPACE_END(net)
LY_NAMESPACE_END
