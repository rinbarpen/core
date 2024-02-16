#include <iostream>

#include <core/config/config.h>
#include <core/net/EventLoop.h>
#include <core/net/tcp/TcpServer.h>
#include <core/util/Library.h>
#include <core/util/Record.h>
#include <core/util/Util.h>
#include <core/util/logger/Logger.h>
#include <core/util/thread/ThreadPool.h>
#include <core/util/time/Clock.h>
#include <core/util/time/Timestamp.h>
#include <core/util/time/TimestampDuration.h>

#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/net/rtsp/RtspPusher.h>
#include <core/multimedia/net/rtsp/RtspServer.h>

#include <fmt/core.h>

using namespace ly;
using namespace lynet;
using namespace ly::literals;
using namespace std::literals;

#include <libyuv.h>
#include <GLFW/glfw3.h>

void rtsp_server_on() {
  GET_LOGGER("net")->setLevel(LogLevel::LINFO);
  GET_LOGGER("multimedia")->setLevel(LogLevel::LINFO);
  EventLoop event_loop;
  auto rtsp_server = RtspServer(&event_loop);
  auto rtsp_pusher = RtspPusher::create(&event_loop);
  rtsp_server.start("0.0.0.0", 8554, 10);
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  rtsp_server.stop();
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


  // LogIniter::getLogger("net", LogLevel::LDEBUG, kDefaultFormatPattern, true,
  //   "net", false);
  // LogIniter::getLogger("system", LogLevel::LDEBUG,
  //   kDefaultFormatPattern, true, "system", false);
  // LogIniter::getLogger(
  //   "multimedia", LogLevel::LDEBUG, kDefaultFormatPattern, true, "multimedia", false);
  //
  // LogIniter::getLogger("net.tcp", LogLevel::LDEBUG, kDefaultFormatPattern, true, "net", false)
  //   ->setLogger(GET_LOGGER("net"));
  // LogIniter::getLogger(
  //   "net.udp", LogLevel::LDEBUG, kDefaultFormatPattern, true, "net", false)
  //   ->setLogger(GET_LOGGER("net"));
  // LogIniter::getLogger(
  //   "system.fiber", LogLevel::LDEBUG, kDefaultFormatPattern, true, "system", false)
  //   ->setLogger(GET_LOGGER("system"));
  // LogIniter::getLogger(
  //   "multimedia", LogLevel::LDEBUG, kDefaultFormatPattern, true, "multimedia", false)
  //   ->setLogger(GET_LOGGER("multimedia"));
  // LogIniter::getLogger(
  //   "multimedia.capture", LogLevel::LDEBUG, kDefaultFormatPattern, true, "multimedia", false)
  //   ->setLogger(GET_LOGGER("multimedia"));
  // LogIniter::getLogger("multimedia.codec", LogLevel::LDEBUG,
  //   kDefaultFormatPattern, true, "multimedia", false)
  //   ->setLogger(GET_LOGGER("multimedia"));
  // LogIniter::getLogger("multimedia.rtsp", LogLevel::LDEBUG,
  //   kDefaultFormatPattern, true, "multimedia", false)
  //   ->setLogger(GET_LOGGER("multimedia"));
  // LogIniter::getLogger("multimedia.rtp", LogLevel::LDEBUG,
  //   kDefaultFormatPattern, true, "multimedia", false)
  //   ->setLogger(GET_LOGGER("multimedia"));
  // LogIniter::getLogger("multimedia.rtcp", LogLevel::LDEBUG,
  //   kDefaultFormatPattern, true, "multimedia", false)
  //   ->setLogger(GET_LOGGER("multimedia"));
  // LogIniter::getLogger("multimedia.rtmp", LogLevel::LDEBUG,
  //   kDefaultFormatPattern, true, "multimedia", false)
  //   ->setLogger(GET_LOGGER("multimedia"));

  fmt::println("{}", LogManager::instance()->toYamlString());


  //rtsp_server_on();
  
  
#ifdef __WIN__
  WSACleanup();
#endif
}
