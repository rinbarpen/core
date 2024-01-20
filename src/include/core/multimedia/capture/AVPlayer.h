#pragma once
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
class AVPlayer
{
public:
  AVPlayer() = default;
  virtual ~AVPlayer() = default;

	virtual void start() = 0;
	virtual void stop() = 0;

  LY_NONCOPYABLE(AVPlayer);

protected:
  virtual void play() = 0;

};
LY_NAMESPACE_END
