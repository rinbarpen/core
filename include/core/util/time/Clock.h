#pragma once

#include <core/util/time/Timestamp.h>
#include <core/util/time/TimestampDuration.h>

LY_NAMESPACE_BEGIN
template <class ClockType>
class Clock
{
public:

  static auto tp() -> typename Timestamp<ClockType>::time_point {
    return ClockType::now();
  }
  static auto now() -> Timestamp<ClockType> {
    return Timestamp<ClockType>();
  }
  static auto infinity() -> int64_t { return -1; }
  static auto zero() -> int64_t { return 0; }

};

// template class Clock<T_high_resolution_clock>;
template class Clock<T_steady_clock>;
template class Clock<T_system_clock>;

static std::string time2datetime(::std::time_t t, const char *dateFmt = "%Y-%m-%d %H:%M:%S")
{
  char buffer[80];
  struct tm tm;

#ifdef __WIN__
  ::localtime_s(&tm, &t);
#elif defined(__LINUX__)
  ::localtime_r(&t, &tm);
#endif

  ::std::strftime(buffer, 80, dateFmt, &tm);
  return std::string(buffer);
}

static std::string time2datetime(
  Timestamp<T_system_clock> timestamp, const char *dateFmt = "%Y-%m-%d %H:%M:%S")
{
  char buffer[80];
  struct tm tm;

  ::std::time_t t = T_system_clock::to_time_t(timestamp.current());
#ifdef __WIN__
  ::localtime_s(&tm, &t);
#elif defined(__LINUX__)
  ::localtime_r(&t, &tm);
#endif

  ::std::strftime(buffer, 80, dateFmt, &tm);
  return std::string(buffer);
}


LY_NAMESPACE_END
