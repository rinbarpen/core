#include "core/util/time/Timestamp.h"

#define DEFAULT_TEMPLATE template <class ClockType>

LY_NAMESPACE_BEGIN

DEFAULT_TEMPLATE
Timestamp<ClockType>::Timestamp() 
  : tp_(ClockType::now()) {}

DEFAULT_TEMPLATE
auto Timestamp<ClockType>::now() -> int64_t {
  return ClockType::now()
      .time_since_epoch()
      .count();
}

template <class ClockType>
auto Timestamp<ClockType>::tp() -> time_point {
  return ClockType::now();
}

DEFAULT_TEMPLATE
void Timestamp<ClockType>::reset() {
  tp_ = ClockType::now();
}
DEFAULT_TEMPLATE
auto Timestamp<ClockType>::current() const -> time_point {
  return tp_;
}
DEFAULT_TEMPLATE
auto Timestamp<ClockType>::count() const -> int64_t {
  return tp_
    .time_since_epoch()
    .count();
}

DEFAULT_TEMPLATE
template <class TimeDurationType>
auto Timestamp<ClockType>::duration(Timestamp<ClockType> begin) const
  -> TimestampDuration<TimeDurationType> 
{
  return ::std::chrono::duration_cast<TimeDurationType>(tp_ - begin);
}

DEFAULT_TEMPLATE
template <class TimeDurationType>
auto Timestamp<ClockType>::operator+(
  TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType> {
  return tp_ + duration.duration();
}

DEFAULT_TEMPLATE
template <class TimeDurationType>
auto Timestamp<ClockType>::operator+=(
  TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType> & {
  tp_ += duration.duration();
  return *this;
}

DEFAULT_TEMPLATE
template <class TimeDurationType>
auto Timestamp<ClockType>::operator-(
  TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType> {
  return tp_ - duration.duration();
}

template <class ClockType>
template <class TimeDurationType>
auto Timestamp<ClockType>::operator-=(
  TimestampDuration<TimeDurationType> duration) -> Timestamp<ClockType> & {
  tp_ -= duration.duration();
  return *this;
}


LY_NAMESPACE_END
