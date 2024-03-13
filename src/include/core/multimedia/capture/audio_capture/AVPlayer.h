#pragma once
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
class AVPlayer
{
public:
  AVPlayer() = default;
  virtual ~AVPlayer() = default;

  virtual bool init() = 0;
  virtual bool destroy() = 0;

	virtual bool start() = 0;
  virtual bool stop() = 0;

  LY_NONCOPYABLE(AVPlayer);
protected:
  virtual bool play() = 0;

};
LY_NAMESPACE_END
