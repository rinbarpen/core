#pragma once
#include "core/util/marcos.h"
#include <core/multimedia/ffmpeg/ffmpeg_util.h>
#include <core/multimedia/capture/AudioCapture.h>

LY_NAMESPACE_BEGIN
class FFmpegAudioCapture : public AudioCapture
{
public:
  FFmpegAudioCapture(AVPlayer *player, AVCapture *capture)
    : AudioCapture(player, capture) {

  }
private:

};
LY_NAMESPACE_END
