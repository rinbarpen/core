#pragma once

#include <chrono>

#include <core/util/time/time.h>
#include <core/util/time/TimestampDuration.h>

LY_NAMESPACE_BEGIN

template <class TimeDurationType>
class TimestampDuration;

template <class ClockType = T_steady_clock>
class Timestamp
{
  LY_CHECK(detail::is_clock_v<ClockType>, "Error ClockType");
public:
  using time_point = typename ClockType::time_point;

  Timestamp()
    : tp_(ClockType::now())
  {}
  Timestamp(time_point tp)
    : tp_(tp)
  {}


  template <class TimeDurationType = std::chrono::milliseconds>
  static auto now() -> int64_t
  {
    return std::chrono::time_point_cast<TimeDurationType>(ClockType::now())
    .time_since_epoch()
    .count();
  }
  static auto tp() -> time_point
  {
    return ClockType::now();
  }

  template <class TimeDurationType>
  auto cast() const -> time_point
  {
    LY_CHECK(detail::is_time_duration_v<TimeDurationType>, "Error time type");
    return std::chrono::time_point_cast<TimeDurationType>(tp_);
  }

  void reset() { tp_ = ClockType::now(); }
  LY_NODISCARD auto current() const -> time_point { return tp_; }
  LY_NODISCARD auto count() const -> int64_t {
    return tp_
      .time_since_epoch()
      .count();
  }

  template <class TimeDurationType>
  auto duration(Timestamp begin) const -> TimestampDuration<TimeDurationType>
  {
    return ::std::chrono::duration_cast<TimeDurationType>(tp_ - begin.current());
  }

  template <typename U>
  auto operator+(TimestampDuration<U> duration) const -> Timestamp<ClockType>
  {
    return tp_ + duration.duration();
  }
  template <typename U>
  auto operator+=(TimestampDuration<U> duration) -> Timestamp<ClockType> &
  {
    tp_ += duration.duration();
    return *this;
  }
  template <typename U>
  auto operator-(TimestampDuration<U> duration) const -> Timestamp<ClockType>
  {
    return tp_ - duration.duration();
  }
  template <typename U>
  auto operator-=(TimestampDuration<U> duration) -> Timestamp<ClockType> &
  {
    tp_ -= duration.duration();
    return *this;
  }

  auto operator<(const Timestamp &rhs) const -> bool
  {
    return tp_.time_since_epoch().count() < rhs.count();
  }
  auto operator>(const Timestamp &rhs) const -> bool
  {
    return !(*this <= rhs);
  }
  auto operator<=(const Timestamp &rhs) const -> bool
  {
    return tp_.time_since_epoch().count() <= rhs.count();
  }
  auto operator>=(const Timestamp &rhs) const -> bool
  {
    return !(*this < rhs);
  }
  auto operator==(const Timestamp &rhs) const -> bool
  {
    return tp_.time_since_epoch().count() == rhs.count();
  }
  auto operator!=(const Timestamp &rhs) const -> bool
  {
    return !(*this == rhs);
  }

private:
  typename ClockType::time_point tp_;
};

// template class Timestamp<T_high_resolution_clock>;
template class Timestamp<T_steady_clock>;
template class Timestamp<T_system_clock>;

LY_NAMESPACE_END

#include <fmt/core.h>
#ifdef FMT_VERSION
# define XX(x)                                                \
template <>                                                 \
struct fmt::formatter<::ly::Timestamp<x>>                   \
{                                                           \
  constexpr auto parse(fmt::format_parse_context &ctx) {     \
   return ctx.begin();                                       \
  }                                                          \
  template <typename FormatContext>                          \
  auto format(const ::ly::Timestamp<x> &obj, FormatContext &ctx) { \
    return fmt::format_to(ctx.out(), "{}", obj.count());      \
  }                                                          \
}

XX(::ly::T_steady_clock);
//XX(::ly::T_high_resolution_clock);
XX(::ly::T_system_clock);
# undef XX

#endif