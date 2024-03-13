#pragma once

#include <core/util/time/Clock.h>
#include <core/multimedia/net/MediaSource.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class H264Source : public MediaSource
{
public:
  static H264Source *create(uint32_t frame_rate);
  virtual ~H264Source();

  void setFrameRate(uint32_t frame_rate) { frame_rate_ = frame_rate; }
  uint32_t getFrameRate() const { return frame_rate_; }

  virtual std::string getMediaDescription(uint16_t port) const override;
  virtual std::string getAttribute() const override;

  bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) override;

  static uint32_t getTimestamp();

private:
  H264Source(uint32_t frame_rate);

private:

  uint32_t frame_rate_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
