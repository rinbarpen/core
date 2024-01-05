#include <cstddef>
#include <iostream>
#include <cmath>
#include <mutex>
#include <tuple>

#include <core/util/string_util.h>
#include <core/util/logger/Logger.h>
#include <core/util/time/Timestamp.h>
#include <core/util/time/TimestampDuration.h>
#include <core/util/time/Clock.h>
#include "core/util/Library.h"
#include "core/util/thread/ThreadPool.h"
#include "range/v3/range/concepts.hpp"
#include <core/util/Record.h>
#include <core/xmarco/xmarco.h>
#include <core/av/ffmpeg/ffmpeg_util.h>

#include <fmt/core.h>
#include <range/v3/range.hpp>
// #include <tl/expected.hpp>

using namespace ly;
using namespace ly::literals;
using namespace std::literals;

void test_logger()
{
  auto test_logger = LogIniter::getLogger("Test", LogLevel::LTRACE, kDefaultFormatPattern, false);

  ILOG_TRACE_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_DEBUG_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_INFO_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_WARN_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_ERROR_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_CRITICAL_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_FATAL_FMT(test_logger, "{}", "x"sv * 3);
}

void test_time()
{
  auto t = Clock<T_system_clock>::now();
  auto ts = Timestamp<T_system_clock>();

  LOG_DEBUG_FMT("Current: {}", time2datetime(ts));
  LOG_DEBUG_FMT("Current: {} <=> {}", t, ts.count());

  std::this_thread::sleep_for(1000ms);

  auto duration = TimestampDuration<>::castTo<T_system_clock>(
    ts.current(), Clock<T_system_clock>::tp());
  auto h = TimestampDuration(1h);
  auto s = h.cast<std::chrono::seconds>();
  LOG_DEBUG_FMT(
    "Duration: {} -> {} -> {}", duration.count(), h.count(), s.count());
}

void test_load_library()
{
  Library library;
  auto loaded = library.load("sqlite3");
  if (loaded) {
    LOG_DEBUG() << "Load sqlite3 successfully";
  } else {
    LOG_DEBUG() << "Load sqlite3 faulty";
  }
}

// a^n =
double my_pow(double base, int n)
{
  if (n == 0) return 1.0;

  bool neg_flag = false;
  if (n < 0) {
    neg_flag = true;
    n = std::abs(n);
  }
  if (n & 1 == 0) base = std::fabs(base);

  double res = 1.0;
  while (n) {
    if (n & 1) res *= base;
    base *= base;
    n >>= 1;
  }
  return neg_flag ? 1.0 / res : res;
}

void test_pow()
{
  Record::record("my_pow_1e8", [](){
    for (int i=1;i<=1e8;++i)
      my_pow(-2.8, 10);
  });
  Record::record("std::pow_1e8", [](){
    for (int i=1;i<=1e8;++i)
      std::pow(-2.8, 10);
  });
  Record::record("my_pow", &my_pow, -2.8, 10);
}


void test_expected()
{
  struct X{};
  // tl::expected<X, std::runtime_error> s;
}

void test_range()
{
  for (auto x : {1,2,3}) {
    fmt::print("{}", x);
  }
}

int main()
{
  std::once_flag once;
  std::call_once(once, &lyffmpeg::reg_ffmpeg, lyffmpeg::RegFlag::ALL);
  ffmpeg::version();
  try {
    ThreadPool pool;
    (void)pool.submit([&](){
      Record::record("test_logger", &test_logger);
      fmt::println("");
    });
    (void)pool.submit([&](){
      Record::record("test_time", &test_time);
      fmt::println("");
    });
  } catch(std::exception &e) {
    LOG_FATAL() << e.what();
  }
}
