#pragma once
#include <functional>
#include <string>

#include <core/multimedia/net/media.h>
#include <core/multimedia/net/rtp/rtp.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class MediaSource
{
public:
  using SendFrameCallback = std::function<bool(MediaChannelId, RtpPacket)>;

  MediaSource() = default;
  virtual ~MediaSource() = default;

  virtual std::string getMediaDescription(uint16_t port = 0) const = 0;
  virtual std::string getAttribute() = 0;

  virtual bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) = 0;

  uint32_t getPayloadType() const { return payload_; }
  uint32_t getClockRate() const { return clock_rate_; }
  MediaType getMediaType() const { return media_type_; }
  void setSendFrameCallback(SendFrameCallback callback) { send_frame_callback_ = callback; }

protected:
  MediaType media_type_;
  uint32_t payload_;
  uint32_t clock_rate_;
  SendFrameCallback send_frame_callback_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
