#include <core/multimedia/net/H264Source.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
H264Source::H264Source(uint32_t frame_rate) :
  MediaSource(),
  frame_rate_(frame_rate)
{
  payload_ = 96;
  media_type_ = H264;
  clock_rate_ = 90000;
}

H264Source* H264Source::createNew(uint32_t frame_rate)
{
  return new H264Source(frame_rate);
}

std::string H264Source::getAttribute()
{
  // "a=rtpmap:$payload H264/$clock_rate"
  return "a=rtpmap:96 H264/90000";
}
std::string H264Source::getMediaDescription(uint16_t port) const
{
  char buf[100] = { 0 };
  sprintf(buf, "m=video %hu RTP/AVP 96", port); // \r\nb=AS:2000
  return std::string(buf);
}

bool H264Source::handleFrame(MediaChannelId channel_id, SimAVFrame frame)
{
  uint8_t *frame_buf = reinterpret_cast<uint8_t*>(frame.data.get());
  uint32_t frame_size = frame.size();

  if (frame.timestamp == 0) {
    frame.timestamp = getTimestamp();
  }

  if (frame_size <= MAX_RTP_PAYLOAD_SIZE) {
    RtpPacket rtp_pkt;
    rtp_pkt.size = frame_size + 4 + RTP_HEADER_SIZE;
    rtp_pkt.type = frame.type;
    rtp_pkt.timestamp = frame.timestamp;
    rtp_pkt.last = 1;
    memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE, frame_buf, frame_size);

    if (send_frame_callback_) {
      if (!send_frame_callback_(channel_id, rtp_pkt)) {
        return false;
      }
    }
  }
  else {
    char FU_A[2] = { 0 };

    FU_A[0] = (frame_buf[0] & 0xE0) | 28;
    FU_A[1] = 0x80 | (frame_buf[0] & 0x1f);

    frame_buf += 1;
    frame_size -= 1;

    while (frame_size + 2 > MAX_RTP_PAYLOAD_SIZE) {
      RtpPacket rtp_pkt;
      rtp_pkt.size = (4 + RTP_HEADER_SIZE + MAX_RTP_PAYLOAD_SIZE);
      rtp_pkt.type = frame.type;
      rtp_pkt.timestamp = frame.timestamp;
      rtp_pkt.last = 0;

      rtp_pkt.data.get()[RTP_HEADER_SIZE + 4] = FU_A[0];
      rtp_pkt.data.get()[RTP_HEADER_SIZE + 4 + 1] = FU_A[1];
      memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE + 2, frame_buf, MAX_RTP_PAYLOAD_SIZE - 2);

      if (send_frame_callback_) {
        if (!send_frame_callback_(channel_id, rtp_pkt))
          return false;
      }

      frame_buf += MAX_RTP_PAYLOAD_SIZE - 2;
      frame_size -= MAX_RTP_PAYLOAD_SIZE - 2;

      FU_A[1] &= ~0x80;
    }

    {
      RtpPacket rtp_pkt;
      rtp_pkt.size = (4 + RTP_HEADER_SIZE + 2 + frame_size);
      rtp_pkt.type = frame.type;
      rtp_pkt.timestamp = frame.timestamp;
      rtp_pkt.last = 1;

      FU_A[1] |= 0x40;
      rtp_pkt.data.get()[RTP_HEADER_SIZE + 4] = FU_A[0];
      rtp_pkt.data.get()[RTP_HEADER_SIZE + 4 + 1] = FU_A[1];
      memcpy(rtp_pkt.data.get() + 4 + RTP_HEADER_SIZE + 2, frame_buf, frame_size);

      if (send_frame_callback_) {
        if (!send_frame_callback_(channel_id, rtp_pkt)) {
          return false;
        }
      }
    }
  }

  return true;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
