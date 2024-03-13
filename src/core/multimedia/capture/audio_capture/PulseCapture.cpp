#include <core/multimedia/capture/audio_capture/PulseCapture.h>

LY_NAMESPACE_BEGIN
bool PulseCapture::init()
{
  AVInputFormat *input_format = av_find_input_format("pulse");
  AVFormatContext *format_ctx = avformat_alloc_context();

  AVDictionary *options;
  av_dict_set_int(&options, "ar", 48000, 0);
  av_dict_set_int(&options, "ac", 2, 0);

  if (avformat_open_input(&format_ctx, "hw:0,0", input_format, &options) < 0) {
    avformat_free_context(format_ctx);
    return false;
  }

  AVCodec *codec;
  int stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
  av_dump_format(format_ctx, stream_index, "hw:0,0", 0);

err:
  if (format_ctx) {
    avformat_close_input(&format_ctx);
    avformat_free_context(format_ctx);
  }
  return false;
}
bool PulseCapture::destroy()
{
  return true;
}

bool PulseCapture::start()
{
  return false;
}
bool PulseCapture::stop()
{
  return true;
}
LY_NAMESPACE_END
