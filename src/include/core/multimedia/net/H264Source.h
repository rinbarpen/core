#pragma once

#include <chrono>
#include <core/multimedia/net/MediaSource.h>
#include <core/util/time/Clock.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class H264Source : public MediaSource
{
public:
  static H264Source *create(uint32_t framerate);
  virtual ~H264Source() = default;

  void setFrameRate(uint32_t framerate) { framerate_ = framerate; }
  uint32_t getFrameRate() const { return framerate_; }

  virtual std::string getAttribute() override;
  virtual std::string getMediaDescription(uint16_t port) const override;

  bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) override;

  static uint32_t getTimestamp()
  {
    auto timestamp = Timestamp<T_steady_clock>::now<std::chrono::milliseconds>();
    return ((timestamp / 1000 + 500) / 1000 * 96);
  }

private:
  H264Source(uint32_t framerate);

private:

  uint32_t framerate_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
