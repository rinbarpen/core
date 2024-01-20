#pragma once

#include <chrono>
#include <core/multimedia/net/MediaSource.h>
#include <core/util/time/Clock.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class H264Source : public MediaSource
{
public:
  static H264Source *createNew(uint32_t frame_rate = 25);
  virtual ~H264Source() = default;

  void setFrameRate(uint32_t frame_rate) { frame_rate_ = frame_rate; }
  uint32_t getFrameRate() const { return frame_rate_; }

  virtual std::string getAttribute() override;
  virtual std::string getMediaDescription(uint16_t port) const override;

  bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) override;

  static uint32_t getTimestamp()
  {
    auto timestamp = Clock<T_steady_clock>::now();
    return ((timestamp.count() / 1000 + 500) / 1000 * 96);
  }

private:
  H264Source(uint32_t frame_rate);

private:

  uint32_t frame_rate_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
