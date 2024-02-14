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


#include <core/util/Util.h>
#include <core/util/Record.h>
#include <core/util/Library.h>
#include <core/util/time/Clock.h>
#include <core/util/time/Timestamp.h>
#include <core/util/time/TimestampDuration.h>
#include <core/util/logger/Logger.h>
#include <core/util/thread/ThreadPool.h>
#include <core/config/config.h>
#include <core/net/EventLoop.h>
#include <core/net/tcp/TcpServer.h>

// #include <core/multimedia/ffmpeg/FFmpegUtil.h>
// #include <core/multimedia/net/rtsp/RtspPusher.h>
// #include <core/multimedia/net/rtsp/RtspServer.h>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <range/v3/algorithm.hpp>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>
#include <range/v3/all.hpp>

using namespace ly;
using namespace lynet;
using namespace ly::literals;
using namespace std::literals;

#include <GLFW/glfw3.h>
#include <libyuv.h>

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
