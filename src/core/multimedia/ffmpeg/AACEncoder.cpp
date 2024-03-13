#include <core/multimedia/ffmpeg/AACEncoder.h>
#include <core/util/logger/Logger.h>

static auto g_ffmpeg_logger = GET_LOGGER("multimedia.ffmpeg");

LY_NAMESPACE_BEGIN
namespace ffmpeg
{
AACEncoder::AACEncoder()
{}
AACEncoder::~AACEncoder()
{
  this->close();
}
bool AACEncoder::prepare(AVConfig &config)
{
  const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
  // const AVCodec *codec = avcodec_find_decoder_by_name("libfdk-aac");
  if (nullptr == codec) {
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[AACEncoder] AAC not found");
    return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);
  if (nullptr == codec_context_) {
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[AACEncoder] Out of memory in allocating codec context");
    return false;
  }

  codec_context_->bit_rate = config.audio.bit_rate;
  codec_context_->channels = config.audio.channels;
  codec_context_->sample_fmt = AV_SAMPLE_FMT_FLTP;
  codec_context_->sample_rate = config.audio.sample_rate;
  codec_context_->channel_layout = av_get_default_channel_layout(config.audio.channels);

  if (::avcodec_open2(codec_context_, codec, 0) != 0) {
    ::avcodec_free_context(&codec_context_);
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[AACEncoder] Fail to open codec");
    return false;
  }

  ILOG_DEBUG_FMT(g_ffmpeg_logger, "[AACEncoder] Initialize successfully");

  AudioInfo in;
  in.sample_rate = config.audio.sample_rate;
  in.channels = config.audio.channels;
  in.bits_per_sample = config.audio.bit_rate;
  in.format = config.audio.format;
  AudioInfo out;
  out.sample_rate = config.audio.sample_rate;
  out.channels = config.audio.channels;
  out.bits_per_sample = config.audio.bit_rate;
  out.format = AV_SAMPLE_FMT_FLTP;

  config_ = config;
  resampler_.reset(new Resampler(in, out));
  return true;
}
void AACEncoder::close()
{
  if (codec_context_) {
    avcodec_free_context(&codec_context_);
    ILOG_DEBUG_FMT(g_ffmpeg_logger, "[AACEncoder] Close successfully");
  }

  pts_ = 0;
}
AVPacketPtr AACEncoder::encode(AVEncodeContext ctx)
{
  AVFramePtr in_frame = makeFramePtr();

  in_frame->format = AV_SAMPLE_FMT_FLTP;
  in_frame->sample_rate = codec_context_->sample_rate;
  in_frame->nb_samples = ctx.samples;
  in_frame->channels = codec_context_->channels;
  in_frame->channel_layout = codec_context_->channel_layout;
  in_frame->pts = av_rescale_q(pts_, { 1, codec_context_->sample_rate }, codec_context_->time_base);
  pts_ += in_frame->nb_samples;

  if (av_frame_get_buffer(in_frame.get(), 0) < 0) {
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[AACEncoder] Out of memory in allocating frame buffer");
    return nullptr;
  }

  int bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_FLTP);
  if (bytes_per_sample == 0) {
    ILOG_ERROR_FMT(g_ffmpeg_logger, "[AACEncoder] Layout AV_SAMPLE_FMT_FLTP is zero bytes, FFmpeg may be broken");
    return nullptr;
  }

  ::memcpy(in_frame->data[0], ctx.pcm, bytes_per_sample * in_frame->channels * ctx.samples);

  AVFramePtr fltp_frame;
  if (resampler_->convert(in_frame, fltp_frame) <= 0) {
    return nullptr;
  }

  int r = avcodec_send_frame(codec_context_, fltp_frame.get());
  if (r < 0) {
    ILOG_WARN_FMT(g_ffmpeg_logger, "[AACEncoder] Error occurs while sending fltp frame");
    return nullptr;
  }

  AVPacketPtr packet = makePacketPtr();
  r = avcodec_receive_packet(codec_context_, packet.get());
  if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
    ILOG_INFO_FMT(g_ffmpeg_logger, "[AACEncoder] No packet to be gotten");
    return nullptr;
  }
  else if (r < 0) {
    ILOG_WARN_FMT(g_ffmpeg_logger, "[AACEncoder] Error occurs while receiving packet");
    return nullptr;
  }

  return packet;
}
}  // namespace ffmpeg
LY_NAMESPACE_END
