#include <core/util/time/Timestamp.h>

LY_NAMESPACE_BEGIN

template <class ClockType>
Timestamp<ClockType>::Timestamp() 
  : tp_(ClockType::now()) {}

template <class ClockType>
Timestamp<ClockType>::Timestamp(time_point tp)
  : tp_(tp)
{}

template <class ClockType>
auto Timestamp<ClockType>::now() -> int64_t {
  return ClockType::now()
      .time_since_epoch()
      .count();
}

template <class ClockType>
auto Timestamp<ClockType>::tp() -> time_point {
  return ClockType::now();
}

template <class ClockType>
void Timestamp<ClockType>::reset() {
  tp_ = ClockType::now();
}

template <class ClockType>
auto Timestamp<ClockType>::current() const -> time_point {
  return tp_;
}

template <class ClockType>
auto Timestamp<ClockType>::count() const -> int64_t {
  return tp_
    .time_since_epoch()
    .count();
}

LY_NAMESPACE_END
