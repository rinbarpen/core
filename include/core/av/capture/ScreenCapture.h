#pragma once
#include <vector>

#include "marcos.h"

class ScreenCapture
{
public:
  ScreenCapture() = default;
  virtual ~ScreenCapture() = default;

  virtual bool init(int display_index = 0) = 0;
  virtual bool destroy() = 0;
  virtual bool captureFrame(std::vector<uint8_t> &image, uint32_t &width, uint32_t &height) = 0;


  virtual bool isCapturing() const = 0;
  virtual uint32_t getWidth() const = 0;
  virtual uint32_t getHeight() const = 0;
  // virtual std::pair<uint32_t, uint32_t> getSize() const = 0;

  LY_NONCOPYABLE(ScreenCapture);
private:

};
