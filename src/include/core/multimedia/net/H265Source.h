#pragma once
#include <core/multimedia/net/media.h>
#include <core/multimedia/net/MediaSource.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class H265Source : public MediaSource
{
public:
  static auto create(uint32_t framerate) -> H265Source*;
  virtual ~H265Source() = default;

  void setFramerate(uint32_t framerate) { framerate_ = framerate; }
  auto getFramerate() const -> uint32_t { return framerate_; }

  auto handleFrame(MediaChannelId channel_id, SimAVFrame frame) -> bool override;

  virtual auto getMediaDescription(uint16_t port) const -> std::string override;
  virtual auto getAttribute() -> std::string override;

  static auto getTimestamp() -> uint32_t;

private:
  H265Source(uint32_t framerate);

private:
  uint32_t framerate_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
