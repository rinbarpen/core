#pragma once

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
enum class CodecId
{
  H264_NV12,
  H264_QSV,
  H264_FFMPEG,
  H265_FFMPEG,
};

LY_NAMESPACE_END
