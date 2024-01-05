#pragma once
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

class AVCapture
{
public:
  AVCapture() = default;
  virtual ~AVCapture() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

	LY_NONCOPYABLE(AVCapture);
protected:
	virtual bool capture() = 0;

};

LY_NAMESPACE_END
