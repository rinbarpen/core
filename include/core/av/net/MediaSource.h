#pragma once
#include <functional>
#include <string>

#include "proto/rtp/rtp.h"
#include "media/media.h"

class MediaSource
{
public:
  using SendFrameCallback = std::function<bool(MediaChannelId, RtpPacket)>;

  MediaSource() {}
  virtual ~MediaSource() {}

  virtual std::string getMediaDescription(uint16_t port = 0) const = 0;
  virtual std::string getAttribute() = 0;

  virtual bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) = 0;

  uint32_t getPayloadType() const { return payload_; }
  uint32_t getClockRate() const { return clock_rate_; }
  MediaType getMediaType() const { return media_type_; }
  void setSendFrameCallback(SendFrameCallback fn) { send_frame_cb_ = fn; }

protected:
  MediaType media_type_;
  uint32_t payload_;
  uint32_t clock_rate_;
  SendFrameCallback send_frame_cb_;
};

