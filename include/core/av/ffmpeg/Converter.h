#pragma once
#include "ffmpeg_util.h"

namespace ffmpeg
{
struct VideoInfo
{
  int height;
  int width;
  AVPixelFormat format;
};
class Converter
{
public:
  Converter(VideoInfo in, VideoInfo out)
  {
    sws_context_ = sws_getContext(in.width, in.height, in.format,
                                  out.width, out.height, out.format, 
                                  SWS_BICUBIC, 0, 0, 0);
    out_video_info_ = out;
  }

  ~Converter()
  {
    if (sws_context_) {
      sws_freeContext(sws_context_);
      sws_context_ = nullptr;
    }
  }

  int convert(AVFramePtr in_frame, AVFramePtr &out_frame)
  {
    out_frame = makeFramePtr();

    out_frame->format  = out_video_info_.format;
    out_frame->height  = out_video_info_.height;
    out_frame->width   = out_video_info_.width;
    out_frame->pts     = in_frame->pts;
    out_frame->pkt_dts = in_frame->pkt_dts;

    if (av_frame_get_buffer(out_frame.get(), 32) != 0) {
      return -1;
    }

    const int out_height = 
      sws_scale(sws_context_, 
        in_frame->data, in_frame->linesize, 0, in_frame->height,
        out_frame->data, out_frame->linesize);

    return out_height;
  }

  Converter(const Converter&) = delete;
  Converter& operator=(const Converter&) = delete;
private:
  SwsContext *sws_context_;
  VideoInfo out_video_info_;
};
}
