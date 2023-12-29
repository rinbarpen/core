#include <core/util/time/TimestampDuration.h>

LY_NAMESPACE_BEGIN

template <class TimeDurationType>
TimestampDuration<TimeDurationType>::TimestampDuration(
  TimeDurationType duration) : duration_(duration)
  {}

template <class TimeDurationType>
TimestampDuration<TimeDurationType>::TimestampDuration(uint64_t duration)
  : duration_(TimeDurationType(duration))
{}

template <class TimeDurationType>
auto TimestampDuration<TimeDurationType>::duration() const -> TimeDurationType {
  return duration_;
}

template <class TimeDurationType>
auto TimestampDuration<TimeDurationType>::count() const -> int64_t {
  return duration_.count();
}


LY_NAMESPACE_END
