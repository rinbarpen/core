#pragma once
#include <memory>

#include "AudioBuffer.h"
#include "AudioCapture.h"

#include "WASAPICapture.h"
#include "WASAPIPlayer.h"

class WinAudioCapture : public AudioCapture
{
public:
  explicit WinAudioCapture(WASAPIPlayer *player, WASAPICapture *capture, size_t buffer_size) :
    AudioCapture(player, capture, buffer_size)
  {}
  ~WinAudioCapture() = default;

private:
  void startCapture() override;
  void stopCapture() override;

};

