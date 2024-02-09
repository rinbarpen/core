#include <algorithm>
#include <array>
#include <cstdint>

#include <core/util/time/Clock.h>
#include <core/multimedia/net/rtp/rtp.h>
#include <core/multimedia/net/AACSource.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
AACSource::AACSource(uint32_t sample_rate, uint32_t channels, bool adts_mode) :
	MediaSource(),
	sample_rate_(sample_rate), channels_(channels),
	adts_mode_(adts_mode)
{
	payload_ = 97;
	media_type_ = AAC;
	clock_rate_ = sample_rate;
}

AACSource* AACSource::create(uint32_t sample_rate, uint32_t channels, bool adts_mode)
{
  return new AACSource(sample_rate, channels, adts_mode);
}

uint32_t AACSource::getTimestamp(uint32_t sample_rate)
{
  auto now = Timestamp<T_steady_clock>::now<std::chrono::milliseconds>();
	return (now + 500) / 1000 * sample_rate / 1000;
}

std::string AACSource::getAttribute()
{
	static std::array<uint32_t, 16> AACSampleRate =
	{
		97000, 88200, 64000, 48000,
		44100, 32000, 24000, 22050,
		16000, 12000, 11025, 8000,
		7350, 0, 0, 0 /*reserved */
	};

	char buf[500] = { 0 };
	sprintf(buf, "a=rtpmap:97 MPEG4-GENERIC/%u/%u\r\n", sample_rate_, channels_);

	auto it = std::find(AACSampleRate.cbegin(), AACSampleRate.cend(), sample_rate_);
	if (it == AACSampleRate.cend()) {
		return ""; // error
	}
	uint8_t index = std::distance(AACSampleRate.cbegin(), it);

	uint8_t profile = 1;
	char config[10] = { 0 };

	sprintf(config, "%02x%02x", (uint8_t)((profile + 1) << 3) | (index >> 1), (uint8_t)((index << 7) | (channels_ << 3)));
	sprintf(buf + strlen(buf),
					"a=fmtp:97 profile-level-id=1;"
					"mode=AAC-hbr;"
					"sizelength=13;indexlength=3;indexdeltalength=3;"
					"config=%04u",
					atoi(config));

	return std::string(buf);
}
std::string AACSource::getMediaDescription(uint16_t port) const
{
  char buf[100] = { 0 };
  sprintf(buf, "m=audio %hu RTP/AVP 97", port); // \r\nb=AS:64
  return std::string(buf);
}

bool AACSource::handleFrame(MediaChannelId channel_id, SimAVFrame frame)
{
	if (frame.size() > (MAX_RTP_PAYLOAD_SIZE - AU_SIZE)) {
		return false;
	}

	int adts_size = 0;
	if (adts_mode_) {
		adts_size = ADTS_SIZE;
	}

	uint8_t *frame_buf = reinterpret_cast<uint8_t*>(frame.data.get() + adts_size);
	uint32_t frame_size = frame.size() - adts_size;

	char AU[AU_SIZE] = { 0 };
	AU[0] = 0x00;
	AU[1] = 0x10;
	AU[2] = (frame_size & 0x1fe0) >> 5;
	AU[3] = (frame_size & 0x1f) << 3;

	RtpPacket rtp_pkt;
	rtp_pkt.size = (frame_size + 4 + RTP_HEADER_SIZE + AU_SIZE);
	rtp_pkt.type = frame.type;
	rtp_pkt.timestamp = frame.timestamp;
	rtp_pkt.last = 1;

	rtp_pkt.data.get()[4 + RTP_HEADER_SIZE + 0] = AU[0];
	rtp_pkt.data.get()[4 + RTP_HEADER_SIZE + 1] = AU[1];
	rtp_pkt.data.get()[4 + RTP_HEADER_SIZE + 2] = AU[2];
	rtp_pkt.data.get()[4 + RTP_HEADER_SIZE + 3] = AU[3];

	memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE + AU_SIZE, frame_buf, frame_size);

	if (send_frame_callback_) {
		send_frame_callback_(channel_id, rtp_pkt);
	}

	return true;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
