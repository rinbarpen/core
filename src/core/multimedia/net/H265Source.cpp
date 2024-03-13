#include <cstdint>
#include <cstring>
#include <core/util/time/Clock.h>
#include <core/multimedia/net/rtp/rtp.h>
#include <core/multimedia/net/H265Source.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
H265Source::H265Source(uint32_t frame_rate)
  : frame_rate_(frame_rate) {
  payload_ = 96;
  media_type_ = MediaType::H265;
  clock_rate_ = 90000;
}
H265Source::~H265Source() {
}

H265Source* H265Source::create(uint32_t frame_rate) {
  return new H265Source(frame_rate);
}
bool H265Source::handleFrame(MediaChannelId channel_id, SimAVFrame frame) {
  auto frame_buf = frame.data.get();
  uint32_t frame_size = frame.data.size();

  if (frame.timestamp <= 0)
  {
    frame.timestamp = H265Source::getTimestamp();
  }

  if (frame_size <= MAX_RTP_PAYLOAD_SIZE)
  {
    RtpPacket rtp_pkt;
    rtp_pkt.size = frame_size + 4 + RTP_HEADER_SIZE;
    rtp_pkt.type = frame.type;
    rtp_pkt.timestamp = frame.timestamp;
    rtp_pkt.last = 1;

    memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE, frame_buf, frame_size);

    if (send_frame_callback_
    && !send_frame_callback_(channel_id, rtp_pkt))
    {
      return false;
    }
    return true;
  }

  char FU[3] = {0};
  char nal_unit_type = (frame_buf[0] & 0x7E) >> 1;
  FU[0] = (frame_buf[0] & 0x81) | (49 << 1);
  FU[1] = frame_buf[1];
  FU[2] = (0x80 | nal_unit_type);

  frame_buf += 2;
  frame_size -= 2;

  while (frame_size + 3 > MAX_RTP_PAYLOAD_SIZE)
  {
    RtpPacket rtp_pkt;
    rtp_pkt.size = 4 + RTP_HEADER_SIZE + MAX_RTP_PAYLOAD_SIZE;
    rtp_pkt.type = frame.type;
    rtp_pkt.timestamp = frame.timestamp;
    rtp_pkt.last = 0;

    rtp_pkt.data.get()[RTP_HEADER_SIZE + 4] = FU[0];
    rtp_pkt.data.get()[RTP_HEADER_SIZE + 5] = FU[1];
    rtp_pkt.data.get()[RTP_HEADER_SIZE + 6] = FU[2];
    memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE + 3, frame_buf,
      MAX_RTP_PAYLOAD_SIZE - 3);

    if (send_frame_callback_
    && !send_frame_callback_(channel_id, rtp_pkt))
    {
      return false;
    }

    frame_buf += (MAX_RTP_PAYLOAD_SIZE - 3);
    frame_size -= (MAX_RTP_PAYLOAD_SIZE - 3);

    FU[2] &= ~0x80;
  }

  {
    RtpPacket rtp_pkt;
    rtp_pkt.type = frame.type;
    rtp_pkt.timestamp = frame.timestamp;
    rtp_pkt.size = 4 + RTP_HEADER_SIZE + 3 + frame_size;
    rtp_pkt.last = 1;

    FU[2] |= 0x40;
    rtp_pkt.data.get()[RTP_HEADER_SIZE + 4] = FU[0];
    rtp_pkt.data.get()[RTP_HEADER_SIZE + 5] = FU[1];
    rtp_pkt.data.get()[RTP_HEADER_SIZE + 6] = FU[2];
    memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE + 3, frame_buf, frame_size);

    if (send_frame_callback_
    && !send_frame_callback_(channel_id, rtp_pkt))
    {
      return false;
    }
  }
  return true;
}

std::string H265Source::getMediaDescription(uint16_t port) const {
  char buf[100] = {0};
  sprintf(buf, "m=video %hu RTP/AVP 96", port);
  return std::string(buf);
}
std::string H265Source::getAttribute() const {
  return std::string("a=rtpmap:96 H265/90000");
}

uint32_t H265Source::getTimestamp() {
  auto timestamp = Timestamp<T_steady_clock>::now<std::chrono::milliseconds>();
  return ((timestamp / 1000 + 500) / 1000 * 90);
}

NAMESPACE_END(net)
LY_NAMESPACE_END
