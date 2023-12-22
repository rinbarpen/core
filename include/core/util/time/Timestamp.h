#pragma once

#include <core/util/time/time.h>
#include <core/util/time/TimestampDuration.h>

LY_NAMESPACE_BEGIN

template <class TimeDurationType>
class TimestampDuration;

template <class ClockType = T_steady_clock>
class Timestamp 
{
  LY_CHECK(is_clock_v<ClockType>, "Error ClockType");
public:
  using time_point = typename ClockType::time_point;
  using default_duration_type = ::std::chrono::milliseconds;

  Timestamp();

  static auto now() -> int64_t;
  static auto tp() -> time_point;

  void reset();
  auto current() const -> time_point;
  auto count() const -> int64_t;

  template <class TimeDurationType = default_duration_type>
  auto duration(Timestamp begin) const -> TimestampDuration<TimeDurationType>;

  template <class TimeDurationType = default_duration_type>
  auto operator+(
    TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType>;
  template <class TimeDurationType = default_duration_type>
  auto operator+=(
    TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType> &;
  template <class TimeDurationType = default_duration_type>
  auto operator-(
    TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType>;
  template <class TimeDurationType = default_duration_type>
  auto operator-=(
    TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType> &;

private:
  time_point tp_;
};

template class Timestamp<T_high_resolution_clock>;
template class Timestamp<T_steady_clock>;
template class Timestamp<T_system_clock>;

LY_NAMESPACE_END
