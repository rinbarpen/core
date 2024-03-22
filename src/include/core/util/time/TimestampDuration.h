#pragma once

#include <core/util/time/Timestamp.h>
#include <core/util/time/time.h>


LY_NAMESPACE_BEGIN
template <class ClockType>
class Timestamp;

template <class TimeDurationType = ::std::chrono::milliseconds>
class TimestampDuration
{
  LY_CHECK(
    detail::is_time_duration_v<TimeDurationType>, "Error TimeDurationType");

public:
  TimestampDuration(TimeDurationType duration) : duration_(duration) {}
  TimestampDuration(uint64_t duration)
    : duration_(TimeDurationType(duration)) {}

  TimeDurationType duration() const { return duration_; }
  int64_t count() const { return duration_.count(); }
  template <class U>
  U cast() const {
    LY_CHECK(detail::is_time_duration_v<U>, "Error TimeDurationType");

    return std::chrono::duration_cast<U>(duration_);
  }

  template <class ClockType>
  static TimestampDuration<TimeDurationType> castTo(
    typename Timestamp<ClockType>::time_point begin,
    typename Timestamp<ClockType>::time_point end) {
    return std::chrono::duration_cast<TimeDurationType>(end - begin);
  }

  template <class ClockType>
  TimestampDuration<TimeDurationType> operator+(
    Timestamp<ClockType> timestamp) const {
    return duration_ + timestamp.currentTp();
  }
  template <class ClockType>
  TimestampDuration<TimeDurationType> &operator+=(
    Timestamp<ClockType> timestamp) {
    duration_ += timestamp.currentTp();
    return *this;
  }
  template <class ClockType>
  TimestampDuration<TimeDurationType> operator-(
    Timestamp<ClockType> timestamp) const {
    return duration_ - timestamp.currentTp();
  }
  template <class ClockType>
  TimestampDuration<TimeDurationType> &operator-=(
    Timestamp<ClockType> timestamp) {
    duration_ -= timestamp.currentTp();
    return *this;
  }

  template <class U>
  bool operator<(const TimestampDuration<U> &rhs) const {
    if constexpr (!std::is_same_v<TimeDurationType, U>) {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_)
             < std::chrono::duration_cast<std::chrono::nanoseconds>(
               rhs.duration_);
    }
    return duration_ < rhs.duration_;
  }
  template <class U>
  bool operator>(const TimestampDuration<U> &rhs) const {
    return !(*this <= rhs);
  }
  template <class U>
  bool operator<=(const TimestampDuration<U> &rhs) const {
    if constexpr (!std::is_same_v<TimeDurationType, U>) {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_)
             <= std::chrono::duration_cast<std::chrono::nanoseconds>(
               rhs.duration_);
    }
    return duration_ <= rhs.duration_;
  }
  template <class U>
  bool operator>=(const TimestampDuration<U> &rhs) const {
    return !(*this < rhs);
  }
  template <class U>
  bool operator==(const TimestampDuration<U> &rhs) const {
    if constexpr (!std::is_same_v<TimeDurationType, U>) {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_)
             == std::chrono::duration_cast<std::chrono::nanoseconds>(
               rhs.duration_);
    }
    return duration_ == rhs.duration_;
  }
  template <class U>
  bool operator!=(const TimestampDuration<U> &rhs) const {
    return !(*this == rhs);
  }

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

#include <fmt/core.h>
#ifdef FMT_VERSION

template <>
struct fmt::formatter<::ly::TimestampDuration<::std::chrono::seconds>>
{
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ::ly::TimestampDuration<::std::chrono::seconds> &obj,
    FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}(s)", obj.count());
  }
};
template <>
struct fmt::formatter<::ly::TimestampDuration<::std::chrono::minutes>>
{
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ::ly::TimestampDuration<::std::chrono::minutes> &obj,
    FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}(min)", obj.count());
  }
};
template <>
struct fmt::formatter<::ly::TimestampDuration<::std::chrono::hours>>
{
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ::ly::TimestampDuration<::std::chrono::hours> &obj,
    FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}(h)", obj.count());
  }
};
template <>
struct fmt::formatter<::ly::TimestampDuration<::std::chrono::nanoseconds>>
{
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ::ly::TimestampDuration<::std::chrono::nanoseconds> &obj,
    FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}(ns)", obj.count());
  }
};
template <>
struct fmt::formatter<::ly::TimestampDuration<::std::chrono::milliseconds>>
{
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ::ly::TimestampDuration<::std::chrono::milliseconds> &obj,
    FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}(ms)", obj.count());
  }
};
template <>
struct fmt::formatter<::ly::TimestampDuration<::std::chrono::microseconds>>
{
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ::ly::TimestampDuration<::std::chrono::microseconds> &obj,
    FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}(us)", obj.count());
  }
};

#endif
