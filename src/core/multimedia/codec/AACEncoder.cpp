#include <core/multimedia/codec/AACEncoder.h>

LY_NAMESPACE_BEGIN

bool AACEncoder::init(int sample_rate, int channels, int format, int bit_rate_bps)
{
	// if (aac_encoder_.getAVCodecContext()) {
	// 	return false;
	// }

	ffmpeg::AVConfig encoder_config;
	encoder_config.audio.sample_rate = sample_rate_ = sample_rate;
	encoder_config.audio.bit_rate = bit_rate_ = bit_rate_bps;
	encoder_config.audio.channels = channels_ = channels;
	encoder_config.audio.format = format_ = (AVSampleFormat)format;

	if (!aac_encoder_.prepare(encoder_config)) {
		return false;
	}

	return true;
}

bool AACEncoder::destroy()
{
	sample_rate_ = 0;
	channels_ = 0;
	bit_rate_ = 0;
	format_ = AV_SAMPLE_FMT_NONE;
	aac_encoder_.close();
  return true;
}

int AACEncoder::getFramesCount() const
{
	// if (!aac_encoder_.GetAVCodecContext()) {
	// 	return -1;
	// }

	return aac_encoder_.getFrameNum();
}
int AACEncoder::getSampleRate() const
{
	return sample_rate_;
}
int AACEncoder::getChannelsCount() const
{
	return channels_;
}

int AACEncoder::getSpecificConfig(uint8_t* buf, int max_buf_size) const
{
	AVCodecContext* codec_context = aac_encoder_.getAVCodecContext();
	if (!codec_context) {
    // no source has been opened
    return -1;
	}

	if (max_buf_size < codec_context->extradata_size) {
		// buffer capacity is not enough
    return -1;
	}

	memcpy(buf, codec_context->extradata, codec_context->extradata_size);
	return codec_context->extradata_size;
}

LY_NAMESPACE_END
