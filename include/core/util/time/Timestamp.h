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

  Timestamp();
  Timestamp(time_point tp);

  static auto now() -> int64_t;
  static auto tp() -> time_point;

  void reset();
  auto current() const -> time_point;
  auto count() const -> int64_t;

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

template class Timestamp<T_high_resolution_clock>;
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