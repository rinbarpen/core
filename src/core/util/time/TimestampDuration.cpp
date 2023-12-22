#include "core/util/time/TimestampDuration.h"

#define DEFAULT_TEMPLATE template <class TimeDurationType>

LY_NAMESPACE_BEGIN

DEFAULT_TEMPLATE
TimestampDuration<TimeDurationType>::TimestampDuration(
  TimeDurationType duration) : duration_(duration)
  {}

DEFAULT_TEMPLATE
auto TimestampDuration<TimeDurationType>::duration() -> TimeDurationType 
{
  return duration_;
}

template <class TimeDurationType>
auto TimestampDuration<TimeDurationType>::count() -> int64_t
{
  return duration_.count();
}

DEFAULT_TEMPLATE
template <class ClockType>
auto TimestampDuration<TimeDurationType>::operator+(
  Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType> {
  return duration_ + timestamp.currentTp();
}

DEFAULT_TEMPLATE
template <class ClockType>
auto TimestampDuration<TimeDurationType>::operator+=(
  Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType> & {
  duration_ += timestamp.currentTp();
  return *this;
}

DEFAULT_TEMPLATE
template <class ClockType>
auto TimestampDuration<TimeDurationType>::operator-(
  Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType> {
  return duration_ - timestamp.currentTp();
}

DEFAULT_TEMPLATE
template <class ClockType>
auto TimestampDuration<TimeDurationType>::operator-=(
  Timestamp<ClockType> timestamp) -> TimestampDuration<TimeDurationType> & {
  duration_ -= timestamp.currentTp();
  return *this;
}

LY_NAMESPACE_END
