#include <core/util/time/Clock.h>
#include <core/multimedia/net/G711ASource.h>
#include <core/multimedia/net/rtp/rtp.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
G711ASource::G711ASource() {
  payload_ = 8;
  media_type_ = PCMA;
  clock_rate_ = 8000;
}
G711ASource::~G711ASource() {

}
G711ASource* G711ASource::create() {
  return new G711ASource{};
}

bool G711ASource::handleFrame(MediaChannelId channel_id, SimAVFrame frame) {
  if (frame.size() > MAX_RTP_PAYLOAD_SIZE) {
    return false;
  }

	auto frame_buf  = frame.data.get();
	uint32_t frame_size = frame.size();

	RtpPacket rtp_pkt;
	rtp_pkt.type = frame.type;
	rtp_pkt.timestamp = frame.timestamp;
	rtp_pkt.size = frame_size + 4 + RTP_HEADER_SIZE;
	rtp_pkt.last = 1;

	memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE, frame_buf, frame_size);

	if (send_frame_callback_
  && !send_frame_callback_(channel_id, rtp_pkt)) {
		return false;
	}

	return true;
}

std::string G711ASource::getAttribute() const {
  return std::string{"a=rtpmap:8 PCMA/8000/1"};
}
std::string G711ASource::getMediaDescription(uint16_t port) const {
	char buf[100] = {0};
	snprintf(buf, 100, "m=audio %hu RTP/AVP 8", port);
	return std::string(buf);
}

uint32_t G711ASource::getTimestamp() {
  auto timestamp = Timestamp<T_steady_clock>::now<std::chrono::microseconds>();
  return (timestamp + 500) / 1000 * 8;
}

NAMESPACE_END(net)
LY_NAMESPACE_END
