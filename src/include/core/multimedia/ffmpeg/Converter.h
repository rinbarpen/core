#pragma once
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN
namespace ffmpeg
{
struct VideoInfo
{
  int height;
  int width;
  AVPixelFormat format;
};
/**
 * @brief Resample the audio(pcm) data
 *
 * @note Throw InitializationException when ffmpeg is loaded or memory is not enough
 *
 */
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

  ~Converter() noexcept
  {
    if (sws_context_) {
      sws_freeContext(sws_context_);
      sws_context_ = nullptr;
    }
  }

  int convert(AVFramePtr in_frame, AVFramePtr &out_frame)
  {
    if (nullptr == out_frame) {
      out_frame = makeFramePtr();
    }

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

  LY_NONCOPYABLE(Converter);
protected:
  SwsContext *sws_context_;
  VideoInfo out_video_info_;
};
}  // namespace ffmpeg
LY_NAMESPACE_END
