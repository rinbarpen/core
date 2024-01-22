#pragma once

#include <iomanip>
#include <sstream>

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
  return time2datetime(T_system_clock::to_time_t(timestamp.current()), dateFmt);
}

static auto datetime2time(
  const std::string &datetime, const char *dateFmt = "%Y-%m-%d %H:%M:%S") -> time_t
{
  std::tm tm = {};
  std::istringstream ss(datetime);
  ss >> std::get_time(&tm, dateFmt);

  if (ss.fail()) {
    return -1;
  }

  return std::mktime(&tm);
}

static auto ctime2timestamp(time_t ctime) -> Timestamp<T_system_clock>
{
  return Timestamp<T_system_clock>(T_system_clock::from_time_t(ctime));
}

LY_NAMESPACE_END
