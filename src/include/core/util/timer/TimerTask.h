#pragma once

#include <core/util/time/Clock.h>
#include <functional>


LY_NAMESPACE_BEGIN

using TimerTaskId = uint32_t;
inline constexpr TimerTaskId kInvalidTimerId = -1;

using namespace std::chrono_literals;
using TimerClock = Clock<T_steady_clock>;
using TimerTimestamp = Timestamp<T_steady_clock>;
using TimerTimestampDuration = TimestampDuration<std::chrono::milliseconds>;

struct TimerTask
{
  using TaskCallback = std::function<void()>;

  TimerTaskId id{kInvalidTimerId};
  TaskCallback callback{};
  bool one_shot{true};

  TimerTimestampDuration interval{0ms};
  TimerTimestamp expire_time;

  TimerTask() { expire_time = TimerClock::now(); }
  TimerTask(TaskCallback _callback, bool _oneShot = true)
    : id(++next_id), callback(_callback), one_shot(_oneShot) {
    expire_time = TimerClock::now();
  }
  TimerTask(TaskCallback _callback, int64_t _interval_ms, bool _oneShot = true)
    : id(++next_id)
    , callback(_callback)
    , one_shot(_oneShot)
    , interval(_interval_ms) {
    expire_time = TimerClock::now() + this->interval;
  }
  TimerTask(TaskCallback _callback, std::chrono::milliseconds _interval,
    bool _oneShot = true)
    : id(++next_id)
    , callback(_callback)
    , one_shot(_oneShot)
    , interval(_interval) {
    expire_time = TimerClock::now() + this->interval;
  }

  void setInterval(std::chrono::milliseconds interval) {
    this->interval = interval;
    expire_time = TimerClock::now() + this->interval;
  }

  bool isExpired(TimerTimestamp now) const { return expire_time < now; }

  void operator()() const {
    if (TimerClock::now() > expire_time) {
      callback();
    }
  }

private:
  static inline TimerTaskId next_id = 0;
};

LY_NAMESPACE_END
