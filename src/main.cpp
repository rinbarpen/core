#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <tuple>


#include <core/util/OSUtil.h>
#include <core/util/Record.h>
#include <core/util/logger/Logger.h>
#include <core/util/string_util.h>
#include <core/util/time/Clock.h>
#include <core/util/time/Timestamp.h>
#include <core/util/time/TimestampDuration.h>
#include "core/util/Library.h"
#include "core/util/thread/ThreadPool.h"
#include "core/config/config.h"
#include <core/net/EventLoop.h>
#include "core/net/tcp/TcpServer.h"
// #include <core/multimedia/ffmpeg/ffmpeg_util.h>
// #include "core/multimedia/net/rtsp/RtspPusher.h"
// #include "core/multimedia/net/rtsp/RtspServer.h"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <range/v3/algorithm.hpp>
#include <range/v3/all.hpp>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>



using namespace ly;
using namespace lynet;
using namespace ly::literals;
using namespace std::literals;

void test_logger() {
  auto test_logger = LogIniter::getLogger(
    "Test", LogLevel::LTRACE, kDefaultFormatPattern, false);

  ILOG_TRACE_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_DEBUG_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_INFO_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_WARN_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_ERROR_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_CRITICAL_FMT(test_logger, "{}", "x"sv * 3);
  ILOG_FATAL_FMT(test_logger, "{}", "x"sv * 3);
}

void test_time() {
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

void test_load_library() {
  Library library;
  auto loaded = library.load("sqlite3");
  if (loaded)
  {
    LOG_DEBUG() << "Load sqlite3 successfully";
  }
  else
  {
    LOG_DEBUG() << "Load sqlite3 faulty";
  }
}

void test_tcp() {
  LogIniter::getLogger("net", LogLevel::LDEBUG, kDefaultFormatPattern, false);

  // LogIniter::loadYamlFile("./config/log.yml");
  // g_config = Config::fromFile("./config/config.yml");

  lynet::EventLoop eventLoop(g_config.common.threadpool.thread_num);

  lynet::TcpServer tcp_server(&eventLoop);
  tcp_server.start("192.168.146.136", 8080, 10);

  while (true)
    ;
}

#include <GLFW/glfw3.h>
#include <libyuv.h>


void init() {
  // ffmpeg init
  // lyffmpeg::reg_ffmpeg(lyffmpeg::RegFlag::ALL);
  //
  log_load_config();
}

void range_test() {
  auto v = ranges::views::iota(1);
  for (auto _ : v | ranges::views::take(10) | ranges::views::reverse
                  | ranges::views::drop(4))
  {
    LOG_DEBUG() << _;
  }
  std::map<int, std::string> map1;
  for (int i = 0; i < 10; ++i) map1[i] = std::to_string(-i);
  for (auto &[k, v] : map1 | ranges::views::take(4) | ranges::views::reverse)
  {
    LOG_DEBUG() << k << " => " << v;
  }

  auto c = ranges::count(v, 6);
  auto b = ranges::any_of(v, [](auto x) { return x == 6; });
  b = ranges::all_of(v, [](auto x) { return x == 6; });
  b = ranges::none_of(v, [](auto x) { return x == 6; });
}

// void rtsp_server_on() {
//   EventLoop event_loop;
//   auto rtsp_server = RtspServer(&event_loop);
//   auto rtsp_pusher = RtspPusher::create(&event_loop);

//   rtsp_server.start("0.0.0.0", 8554, 10);
//   while (true)
//   {
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//   }
//   rtsp_server.stop();
// }

void threadpool_test() {
  try
  {
    ThreadPool pool;
    (void) pool.submit([&]() {
      Record::record("test_logger", &test_logger);
      fmt::println("");
    });
    (void) pool.submit([&]() {
      Record::record("test_time", &test_time);
      fmt::println("");
    });
  }
  catch (std::exception &e)
  { LOG_FATAL() << e.what(); }
}

int main() {
#ifdef __WIN__
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    return -1;
  }
#endif
#ifdef LIBYUV_VERSION
  fmt::println("Libyuv is loaded");
#endif
#ifdef GLFW_VERSION_MAJOR
  fmt::println("GLFW is loaded");
#endif
  LogIniter::getLogger("net", LogLevel::LDEBUG, kDefaultFormatPattern, false);


  // os_api::mk("config/log.yml");
  // LogIniter::loadYamlFile("config/log.yml");
  // fmt::println("{}", LogManager::instance()->toYamlString());
  // LogManager::instance()->toYamlFile("config/log.yml");

#ifdef __WIN__
  WSACleanup();
#endif
}
