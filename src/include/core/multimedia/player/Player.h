#pragma once

#include <cstdint>
#include <core/util/logger/Logger.h>
#include <core/util/thread/Thread.h>
#include <core/multimedia/util/AVQueue.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/ffmpeg/H264Encoder.h>
#include <core/multimedia/ffmpeg/AACEncoder.h>
// #include <core/multimedia/util/AVThread.h>

// #include <SDL2/SDL.h>

LY_NAMESPACE_BEGIN
struct PlayerConfig {
  struct video {
    uint32_t frame_rate = 25;
    uint32_t width = 0, height = 0;  // 0 for unknown
    AVPixelFormat format = AV_PIX_FMT_NONE;
    uint32_t bit_rate = 0;  // 0 for unknown
  } video;
  struct audio {
    AVSampleFormat format = AV_SAMPLE_FMT_NONE;
    uint32_t bit_rate = 0;  // 0 for unknown
    uint32_t volume = 100;
    bool muted = false;
  } audio;
  struct common {
    float speed = 1.0f;
    bool video_on = true;
    bool audio_on = true;
  } common;
};

enum class PlayerStatus {
  NO_SOURCE,  // no source
  PLAYING,
  PAUSE,
};

class Player
{
public:
  Player();
  ~Player();

  bool prepare(PlayerConfig config);
  bool close();

  bool openUrl(std::string_view url);

  void play();      // playing_ -> true if playing_ == false, pausing_ -> false if playing_ == true
  void pause();     // require playing_ == true, pausing_ -> true
  void teardown();  // playing_ -> false

  void seek(int64_t pos); // us
  void setMute(bool muted) { config_.audio.muted = muted; }
  void setVolume(float volume) { config_.audio.volume = volume; }
  float getVolume() const { return config_.audio.volume; }

  int64_t getTotalTime() const;  // us
  double getCurrentTime() const;  // s

private:

private:
  void onDecodeVideo();
  void onDecodeAudio();

private:
  PlayerConfig config_;
  bool initialized_{false};
  PlayerStatus status_;

  std::string url_;  // url or filename
  bool opened_{false};  // set while opening an url
  bool sync_by_audio_{true}; // use video to sync if false, use audio to sync if true

  int video_stream_index_{-1};
  int audio_stream_index_{-1};
  AVCodecID vcodec_id_, acodec_id_;

  AVPacketQueue video_in_;
  AVFrameQueue video_out_;
  AVPacketQueue audio_in_;
  AVFrameQueue audio_out_;

  Thread video_decoder_thread_{"video"};
  Thread audio_decoder_thread_{"audio"};

  // extensions
  bool record_while_playing_{false};
};
LY_NAMESPACE_END
