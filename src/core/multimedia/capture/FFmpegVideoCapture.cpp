#include <core/multimedia/capture/FFmpegScreenCapture.h>
#include <core/util/logger/Logger.h>
#include <libavdevice/avdevice.h>

LY_NAMESPACE_BEGIN
static auto g_multimedia_logger = GET_LOGGER("multimedia");
FFmpegScreenCapture::FFmpegScreenCapture(const char *vcodec)
{
  // avdevice_register_all();

}

FFmpegScreenCapture::~FFmpegScreenCapture()
{

}

bool FFmpegScreenCapture::init(int display_index)
{
  if (initialized_) { return true; }

}

bool destroy()
{

}
bool captureFrame(std::vector<uint8_t> &image, uint32_t &width, uint32_t &height)
{

}

LY_NAMESPACE_END
