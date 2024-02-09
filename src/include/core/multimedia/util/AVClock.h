#pragma once

#include <chrono>
#include <core/util/time/time.h>
#include <core/util/time/Clock.h>
#include <core/util/time/Timestamp.h>
#include <cstdint>
#include <thread>

LY_NAMESPACE_BEGIN
class AVClock
{
public:
  using ClockType = T_steady_clock;
  using TimestampType = Timestamp<ClockType>;

  AVClock()
    : current_(TimestampType{})
  {}

  void reset()
  {
    current_ = TimestampType();
  }

  auto elapse() -> std::chrono::milliseconds
  {
    auto now = Clock<T_steady_clock>::now();
    auto duration = now.duration<std::chrono::milliseconds>(current_);
    // this->reset();
    return duration.duration();
  }

  static auto now() -> int64_t
  {
    return TimestampType::now();
  }
  static void sleep(std::chrono::milliseconds sleep_time)
  {
    std::this_thread::sleep_for(sleep_time);
  }

private:
  TimestampType current_;
};

LY_NAMESPACE_END
