#pragma once
#include <cstdint>
#include <memory>
#include <core/multimedia/ffmpeg/ffmpeg_util.h>
#include <core/multimedia/capture/AudioCapture.h>

LY_NAMESPACE_BEGIN
class FFmpegAudioCapture final : public AudioCapture
{
public:
  FFmpegAudioCapture(std::unique_ptr<AVPlayer> player, std::unique_ptr<AVCapture> capture);

private:
  bool startCapture() override;
  bool stopCapture() override;

};
LY_NAMESPACE_END
