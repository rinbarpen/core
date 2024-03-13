#pragma once

#include <cstdint>
#include <chrono>
#include <thread>

#include <core/util/time/time.h>
#include <core/util/time/Clock.h>
#include <core/util/time/Timestamp.h>

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
  static void sleepMS(int sleep_time)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds{sleep_time});
  }

private:
  TimestampType current_;
};

LY_NAMESPACE_END
