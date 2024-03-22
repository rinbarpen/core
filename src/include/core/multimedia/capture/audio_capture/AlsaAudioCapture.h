#pragma once
#include <core/multimedia/capture/audio_capture/AlsaCapture.h>
#include <core/multimedia/capture/audio_capture/AlsaPlayer.h>
#include <core/multimedia/capture/audio_capture/AudioCapture.h>

LY_NAMESPACE_BEGIN
class AlsaAudioCapture : public AudioCapture
{
public:
  AlsaAudioCapture();
  ~AlsaAudioCapture();

private:
  bool startCapture() override;
  bool stopCapture() override;

private:

};
LY_NAMESPACE_END
