#pragma once

#include "media/MediaSource.h"
#include "util/Clock.h"

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
    auto tp = Clock::now<T_steady_clock>();
    return ((tp + 500) / 1000 * 96);
  }

private:
  H264Source(uint32_t frame_rate);

private:

  uint32_t frame_rate_;
};

