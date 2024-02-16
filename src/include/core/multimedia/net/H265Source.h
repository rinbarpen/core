#pragma once
#include <core/multimedia/net/media.h>
#include <core/multimedia/net/MediaSource.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class H265Source : public MediaSource
{
public:
  static H265Source *create(uint32_t framerate);
  virtual ~H265Source() = default;

  void setFrameRate(uint32_t framerate) { framerate_ = framerate; }
  uint32_t getFrameRate() const { return framerate_; }

  bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) override;

  virtual std::string getMediaDescription(uint16_t port) const override;
  virtual std::string getAttribute() override;

  static uint32_t getTimestamp();

private:
  H265Source(uint32_t framerate);

private:
  uint32_t framerate_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
