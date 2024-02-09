#pragma once

#include <core/multimedia/ffmpeg/H264Encoder.h>
#include <vector>

LY_NAMESPACE_BEGIN
enum CodecId
{
  H264_NV12,
  H264_QSV,
  H264_FFMPEG,
};
/**
 * @brief use nv12 first, second is d3dx, third is ffmpeg
 *
 */
class H264Encoder
{
public:
  explicit H264Encoder(CodecId h264CodecId = CodecId::H264_FFMPEG);
  H264Encoder(std::string_view h264CodecName);
  ~H264Encoder() = default;

  void setCodec(CodecId h264CodecId);
  void setCodec(std::string_view h264CodecName);

  bool prepare(int frame_rate, int bit_rate_bps, int format, int width, int height);
  void close();

  int encode(uint8_t* in_buffer, uint32_t in_width, uint32_t in_height,
			   uint32_t image_size, std::vector<uint8_t>& out_frame);

  int getSequenceParams(uint8_t* out_buffer, int out_buffer_size);
  std::string getCodecName() const;

private:
	bool isKeyFrame(const uint8_t* data, uint32_t size);

private:
  bool use_ffmpeg_{false};
  bool use_nv_{false};
  bool use_dxd_{false};

  CodecId codec_id_;
  ffmpeg::AVConfig encoder_config_;
  ffmpeg::H264Encoder::ptr h264_encoder_;
	void* nvenc_data_ = nullptr;
	// QsvEncoder qsv_encoder_;
};
LY_NAMESPACE_END
