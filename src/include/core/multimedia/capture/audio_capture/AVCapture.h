#pragma once
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

struct AudioFormat
{
	int channels;
  int samples_per_sec;
	int bits_per_sample;
};

class AVCapture
{
public:
  AVCapture() = default;
  virtual ~AVCapture() = default;

  virtual bool init() = 0;
  virtual bool destroy() = 0;

  virtual bool start() = 0;
  virtual bool stop() = 0;

  virtual AudioFormat getAudioFormat() const = 0;

	LY_NONCOPYABLE(AVCapture);
protected:
	virtual bool capture() = 0;
};
LY_NAMESPACE_END
