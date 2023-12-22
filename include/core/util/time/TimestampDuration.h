#pragma once

#include <core/util/time/time.h>
#include <core/util/time/Timestamp.h>

LY_NAMESPACE_BEGIN

template <class ClockType>
class Timestamp;

template <class TimeDurationType = ::std::chrono::milliseconds>
class TimestampDuration
{
  LY_CHECK(is_time_duration_v<TimeDurationType>, "Error TimeDurationType");

public:
  TimestampDuration(TimeDurationType duration);

  auto duration() -> TimeDurationType;
  auto count() -> int64_t;
  template <class OtherTimeDurationType>
  auto cast() -> OtherTimeDurationType
  {
    LY_CHECK(is_time_duration_v<TimeDurationType>, "Error TimeDurationType");

    return std::chrono::duration_cast<OtherTimeDurationType>(duration_);
  }

  template <class ClockType>
  static auto castTo(
    typename Timestamp<ClockType>::time_point begin,
    typename Timestamp<ClockType>::time_point end)
    -> TimestampDuration
  {
    return std::chrono::duration_cast<TimeDurationType>(end - begin);
  }

  template <class ClockType>
  auto operator+(
    Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType>;
  template <class ClockType>
  auto operator+=(
    Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType> &;
  template <class ClockType>
  auto operator-(
    Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType>;
  template <class ClockType>
  auto operator-=(
    Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType> &;

private:
  TimeDurationType duration_;
};

template class TimestampDuration<::std::chrono::seconds>;
template class TimestampDuration<::std::chrono::microseconds>;
template class TimestampDuration<::std::chrono::milliseconds>;
template class TimestampDuration<::std::chrono::nanoseconds>;
template class TimestampDuration<::std::chrono::hours>;
template class TimestampDuration<::std::chrono::minutes>;

LY_NAMESPACE_END
