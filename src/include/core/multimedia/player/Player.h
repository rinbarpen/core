#pragma once

#include "core/util/logger/Logger.h"
#include "core/util/marcos.h"
#include "core/multimedia/util/AVQueue.h"
#include <core/multimedia/ffmpeg/H264Encoder.h>
#include <core/multimedia/ffmpeg/AACEncoder.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/rational.h>
#include <memory>
#include <atomic>

LY_NAMESPACE_BEGIN
class Player
{
public:

  auto openFile(std::string_view filename) -> bool
  {
    auto g_multimedia_logger = GET_LOGGER("multimedia");
    // check if the stream has been existed
    AVFormatContext *format_context = avformat_alloc_context();
    int r{0};
    r = avformat_open_input(&format_context, filename.data(), 0, 0);
    if (r < 0) {
      // error url
      ILOG_ERROR_FMT(g_multimedia_logger, "Error opening the file: {}", filename);
      return r;
    }
    r = avformat_find_stream_info(format_context, 0);
    if (r < 0) {
      ILOG_ERROR_FMT(g_multimedia_logger, "Could not find stream information");
      return r;
    }

    if (g_config.multimedia.ffmpeg.open_debug)
      av_dump_format(format_context, 0, filename.data(), 0);

    video_stream_index_ = -1;
    audio_stream_index_ = -1;
    for (int i = 0; i < format_context->nb_streams; i++) {
      if (video_stream_index_ == -1
        && AVMEDIA_TYPE_VIDEO == format_context->streams[i]->codecpar->codec_type) {
        video_stream_index_ = i;
      }
      else if (audio_stream_index_ == -1
        && AVMEDIA_TYPE_AUDIO == format_context->streams[i]->codecpar->codec_type) {
        audio_stream_index_ = i;
      }
    }

    AVCodecParameters *video_codec_parameters{nullptr}, *audio_codec_parameters{nullptr};
    const AVCodec *vcodec{nullptr}, *acodec{nullptr};
    if (video_stream_index_ == -1) {
      ILOG_WARN_FMT(g_multimedia_logger, "Could not find video stream in the input file");
    } else {
      video_codec_parameters = format_context->streams[video_stream_index_]->codecpar;
      vcodec = avcodec_find_decoder(video_codec_parameters->codec_id);
      AVCodecContext *codec_context = avcodec_alloc_context3(vcodec);
      if (!codec_context) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 1;
      }
      if (avcodec_parameters_to_context(codec_context, video_codec_parameters) < 0) {
          fprintf(stderr, "Could not copy codec parameters to codec context\n");
          return 1;
      }
      if (avcodec_open2(codec_context, vcodec, 0) < 0) {
          fprintf(stderr, "Could not open video codec\n");
          return 1;
      }

    }
    if (audio_stream_index_ == -1) {
      ILOG_WARN_FMT(g_multimedia_logger, "Could not find audio stream in the input file");
    } else {
      audio_codec_parameters = format_context->streams[audio_stream_index_]->codecpar;
      acodec = avcodec_find_decoder(audio_codec_parameters->codec_id);
      AVCodecContext *codec_context = avcodec_alloc_context3(vcodec);
      if (!codec_context) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 1;
      }
      if (avcodec_parameters_to_context(codec_context, audio_codec_parameters) < 0) {
          fprintf(stderr, "Could not copy codec parameters to codec context\n");
          return 1;
      }
      if (avcodec_open2(codec_context, acodec, 0) < 0) {
          fprintf(stderr, "Could not open audio codec\n");
          return 1;
      }
    }

    if (nullptr == vcodec) {
      ILOG_WARN_FMT(g_multimedia_logger, "Could not find decoder for the video stream\n");
    }
    if (nullptr == acodec) {
      ILOG_WARN_FMT(g_multimedia_logger, "Could not find decoder for the video stream\n");
    }
  }
  auto openUrl(std::string_view url) -> bool;

  void play();
  void pause();
  void teardown();

private:
  int video_stream_index_{-1};
  int audio_stream_index_{-1};

  AVFrameQueue in_;
  std::unique_ptr<ffmpeg::Encoder> video_encoder;
  std::unique_ptr<ffmpeg::Encoder> audio_encoder;

  bool sync_by_audio_{true}; // use video to sync if false, use audio to sync if true
  std::string url_;  // url or filename
  bool opened_{false};  // set while opening an url
  std::atomic_bool playing_{false};
  bool record_while_playing_{false};
};
LY_NAMESPACE_END
