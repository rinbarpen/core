#pragma once
#include <core/multimedia/net/MediaSource.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class AACSource : public MediaSource
{
public:
  static AACSource *create(uint32_t sample_rate = 44100, uint32_t channels = 2, bool adts_mode = true);
  virtual ~AACSource() = default;

  bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) override;
  virtual std::string getMediaDescription(uint16_t port) const override;
  virtual std::string getAttribute() const override;

  uint32_t getSampleRate() const { return sample_rate_; }

  static uint32_t getTimestamp(uint32_t sample_rate = 44100);

private:
  AACSource(uint32_t sample_rate, uint32_t channels, bool adts_mode);

private:
  uint32_t sample_rate_;
  uint32_t channels_;
  bool adts_mode_;

  static constexpr int ADTS_SIZE = 7;
  static constexpr int AU_SIZE = 4;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
