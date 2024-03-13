#include <iostream>

#include <core/net/EventLoop.h>
#include <core/net/tcp/TcpServer.h>
#include <core/util/Library.h>
#include <core/util/Record.h>
#include <core/util/Util.h>
#include <core/util/logger/Logger.h>
#include <core/util/thread/Thread.h>
#include <core/util/time/Clock.h>
#include <core/util/time/Timestamp.h>
#include <core/util/time/TimestampDuration.h>

//#include <ScreenLive.h>

#include <fmt/core.h>
#include <core/multimedia/codec/encoder/AACEncoder.h>
#include <core/multimedia/codec/encoder/H264Encoder.h>
#include <thread>
#include "ScreenLive.h"
#include "core/multimedia/ffmpeg/FFmpegUtil.h"
#include "core/multimedia/net/rtsp/RtspServer.h"
#include "core/multimedia/net/rtsp/RtspPusher.h"
#include "core/multimedia/net/AACSource.h"
#include "core/multimedia/net/H264Source.h"
#include "core/multimedia/net/MediaSession.h"
#include "core/multimedia/net/media.h"

using namespace ly;
using namespace lynet;
using namespace ly::literals;
using namespace std::literals;

void rtsp_server_on() {
//  GET_LOGGER("net")->setLevel(LogLevel::LINFO);
//  GET_LOGGER("multimedia")->setLevel(LogLevel::LINFO);
  EventLoop event_loop;

  auto rtsp_server = RtspServer::create(&event_loop);
  {
    auto session = MediaSession::create("live/test");
    session->addSource(ly::net::channel_0, H264Source::create(25));
    session->addSource(ly::net::channel_1, AACSource::create(48000, 2, false));
    auto sessionId = rtsp_server->addSession(session);
  }
  ILOG_INFO_FMT(GET_LOGGER("multimedia.rtsp"), "{ip}:{port}{suffix}",
    fmt::arg("ip", "0.0.0.0"), fmt::arg("port", 8554), fmt::arg("suffix", "/live/test"));
  rtsp_server->start("0.0.0.0", 8554, 10);

  // auto rtsp_pusher = RtspPusher::create(&event_loop);
  // {
  //   MediaSession *session = MediaSession::create();
  //   session->addSource(channel_0, H264Source::create(25));
  //   session->addSource(channel_1, AACSource::create(48000, 2, false));

  //   rtsp_pusher->addSession(session);
  //   if (!rtsp_pusher->openUrl("rtsp://127.0.0.1/live/test", 1000ms)) {
  //     rtsp_pusher = nullptr;
  //   }
  // }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  rtsp_server->stop();
}

int main() {
#ifdef __WIN__
  ::WSADATA wsaData;
  if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
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

  // log_load_config();

  LogIniter::reg("system.debugger", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("system.console", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("system.fiber", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("net.tcp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("net.udp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("multimedia.rtp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("multimedia.rtsp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("multimedia.rtmp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("multimedia.codec", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("multimedia.ffmpeg", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("multimedia.capture", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  LogIniter::reg("app.screen.live", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);

  log_export_config();

  auto debugger_logger = GET_LOGGER("system.debugger");
  ILOG_INFO(debugger_logger) << LogManager::instance()->toYamlString();

  ffmpeg::reg_ffmpeg(ffmpeg::RegFlag::ALL);

#if 1
  {
    ScreenLive live;
    ScreenLiveConfig config;
    config.frame_rate = 25;
    config.bit_rate_bps = 800 * 1000;
    config.codec = "x264";
    config.gop = 25;

    live.init(config);
    LiveConfig live_config;
    live_config.server.ip = "127.0.0.1";
    live_config.server.port = 8554;
    live_config.server.suffix = "/live";
    live_config.pusher.rtsp_url = "rtsp://127.0.0.1/live/test";
    live_config.pusher.rtmp_url = "rtmp://127.0.0.1/live/test";
    live.startLive(ScreenLiveType::RTSP_SERVER, live_config);
    live.startLive(ScreenLiveType::RTSP_PUSHER, live_config);
    live.startLive(ScreenLiveType::RTMP_PUSHER, live_config);

    while (true) std::this_thread::sleep_for(1000ms);
  }
#endif

#ifdef __WIN__
  ::WSACleanup();
#endif
}
