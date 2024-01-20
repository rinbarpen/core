#pragma once

#include <chrono>
#include <ctime>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

using T_steady_clock = ::std::chrono::steady_clock;
using T_system_clock = ::std::chrono::system_clock;
using T_high_resolution_clock = ::std::chrono::high_resolution_clock;

template <class ClockType>
struct is_clock : public is_any_of<ClockType, T_steady_clock, T_system_clock, T_high_resolution_clock> {
};
template <class ClockType>
using is_clock_t = typename is_clock<ClockType>::type;
template <class ClockType>
inline constexpr bool is_clock_v = is_clock<ClockType>::value;

template <class TimeDurationType>
struct is_time_duration : public is_any_of<TimeDurationType, ::std::chrono::seconds, ::std::chrono::microseconds, ::std::chrono::milliseconds, ::std::chrono::nanoseconds, ::std::chrono::hours, ::std::chrono::minutes> {
};
template <class TimeDurationType>
using is_time_duration_t = typename is_time_duration<TimeDurationType>::type;
template <class TimeDurationType>
inline constexpr bool is_time_duration_v = is_time_duration<TimeDurationType>::value;

// using time_t = int64_t;

LY_NAMESPACE_END
