#pragma once

#include <core/multimedia/capture/audio_capture/AudioCapture.h>
#include <core/multimedia/capture/audio_capture/WASAPICapture.h>
#include <core/multimedia/capture/audio_capture/WASAPIPlayer.h>

LY_NAMESPACE_BEGIN
class WASAPIAudioCapture : public AudioCapture
{
public:
  WASAPIAudioCapture();
  ~WASAPIAudioCapture() override;

private:
  bool startCapture() override;
  bool stopCapture() override;

};

LY_NAMESPACE_END
