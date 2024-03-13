#include <core/util/logger/Logger.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/ffmpeg/Resampler.h>
#include <core/multimedia/util/Encoder.h>
#include <core/multimedia/util/AACEncoder.h>

#if 0
LY_NAMESPACE_BEGIN
static auto g_encoder_logger = GET_LOGGER("multimedia.coder.encoder");

AACEncoder::AACEncoder(){}
AACEncoder::~AACEncoder(){
  (void)this->close();
}
bool AACEncoder::open(ffmpeg::AVConfig &config)
{
  auto pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
  codec_context_ = avcodec_alloc_context3(pCodec);

  codec_context_->sample_fmt = config.audio.format;
  codec_context_->bit_rate = config.audio.bit_rate;
  codec_context_->channels = config.audio.channels;
  codec_context_->sample_rate = config.audio.sample_rate;
  codec_context_->channel_layout = av_get_default_channel_layout(config.audio.channels);

  avcodec_open2(codec_context_, pCodec, nullptr);

  config_ = config;
  return true;
}
bool AACEncoder::close()
{
  if (codec_context_) {
    avcodec_close(codec_context_);
    avcodec_free_context(&codec_context_);
    codec_context_ = nullptr;
    ILOG_DEBUG_FMT(g_encoder_logger, "[AACEncoder] Close successfully");
  }

  pts_ = 0;
  return true;
}
ffmpeg::AVPacketPtr AACEncoder::encode(ffmpeg::AVFramePtr pFrame, EncoderConfig config)
{
  ffmpeg::AudioInfo in;
  ffmpeg::AudioInfo out;
  in.channels = pFrame->channels;
  in.bits_per_sample = pFrame->nb_samples;
  in.format = (enum AVSampleFormat)pFrame->format;
  in.sample_rate = pFrame->sample_rate;
  out.channels = config.audio.nchannels;
  out.bits_per_sample = config_.audio.bit_rate;  // no trans
  out.format = config.audio.format;
  out.sample_rate = config_.audio.sample_rate;  // no trans
  if ((last_audio_info_in_ != in)
   || (last_audio_info_out_ != out)) {
    // config has been modified
    ILOG_INFO_FMT(g_encoder_logger, "[AACEncoder] The Resampler has been modified. From {} to {}", in, out);
    resampler_.reset(new ffmpeg::Resampler(in, out));
  }

  ffmpeg::AVFramePtr pOutFrame;
  if (resampler_) {
    if (resampler_->convert(pFrame, pOutFrame) < 0) {
      ILOG_ERROR_FMT(g_encoder_logger, "[AACEncoder] Fail to converts with Resampler\nin:\n{}\nout:\n{}", in, out);
      return nullptr;
    }
    last_audio_info_in_ = in;
    last_audio_info_out_ = out;
  }
  else {
    pOutFrame = pFrame;
  }

  int r = avcodec_send_frame(codec_context_, pOutFrame.get());
  if (r < 0) {
    ILOG_WARN_FMT(g_encoder_logger, "[AACEncoder] Error occurs while sending frame");
    return nullptr;
  }

  ffmpeg::AVPacketPtr pPkt = ffmpeg::makePacketPtr();
  r = avcodec_receive_packet(codec_context_, pPkt.get());
  if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
    ILOG_INFO_FMT(g_encoder_logger, "[AACEncoder] No packet to be gotten");
    return nullptr;
  }
  else if (r < 0) {
    ILOG_WARN_FMT(g_encoder_logger, "[AACEncoder] Error occurs while receiving packet");
    return nullptr;
  }

  return pPkt;
}

LY_NAMESPACE_END
#endif
