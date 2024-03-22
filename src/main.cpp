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
#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/config/config.h>
#include <ScreenLive.h>

#include <core/multimedia/codec/encoder/AACEncoder.h>
#include <core/multimedia/codec/encoder/H264Encoder.h>
#include <fmt/core.h>
#include <thread>
#include "core/multimedia/net/rtmp/RtmpServer.h"

using namespace ly;
using namespace lynet;
using namespace ly::literals;
using namespace std::literals;

/*
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
*/

#include <alsa/asoundlib.h>
void test_alsa()
{
  auto s_debugger_logger = GET_LOGGER3("system.debugger");
  auto s_ffmpeg_debug = LY_CONFIG_GET(multimedia.ffmpeg.open_debug);

  auto input_format_ = av_find_input_format("alsa");
	if (nullptr == input_format_) {
		ILOG_ERROR(s_debugger_logger) << "Alsa not found.";
		return ;
	}
  auto format_context_ = avformat_alloc_context();

  if (avformat_open_input(&format_context_, "hw:0,0", input_format_, 0) < 0) {
		ILOG_ERROR(s_debugger_logger) << "Fail to open alsa.";
    avformat_close_input(&format_context_);
    return ;
  }

	if (avformat_find_stream_info(format_context_, nullptr) < 0) {
		ILOG_ERROR(s_debugger_logger) << "Couldn't find stream info.";
    avformat_close_input(&format_context_);
		return ;
	}

  AVCodec *codec;
  int stream_index = av_find_best_stream(format_context_, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
  if (stream_index < 0) {
    ILOG_ERROR(s_debugger_logger) << "No source";
		avformat_close_input(&format_context_);
		return ;
  }
  if (s_ffmpeg_debug)
    av_dump_format(format_context_, stream_index, "hw:0,0", 0);

	auto codec_context_ = avcodec_alloc_context3(codec);
	if (nullptr == codec_context_) {
    ILOG_ERROR(s_debugger_logger) << "Out of memory in allocating codec context";
		avformat_close_input(&format_context_);
		return ;
	}

	avcodec_parameters_to_context(codec_context_, format_context_->streams[stream_index]->codecpar);
	if (avcodec_open2(codec_context_, codec, nullptr) != 0) {
		avcodec_close(codec_context_);
		avformat_close_input(&format_context_);
		return ;
	}

  uint32_t frames = 0;
  auto frame = ffmpeg::makeFramePtr();
  while (true) {
    if (avcodec_receive_frame(codec_context_, frame.get()) < 0) {
      break;
    }
    frames++;
    if (frames % 20 == 0) {
      ILOG_DEBUG(s_debugger_logger) << frames;
    }
  }
}

void start_screenlive()
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
  // live.startLive(ScreenLiveType::RTMP_PUSHER, live_config);

  while (true) std::this_thread::sleep_for(100ms);
}

int main() {
#ifdef __WIN__
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    exit(-1);
  }
#endif
#ifdef LIBYUV_VERSION
  fmt::println("Libyuv is loaded");
#endif
#ifdef GLFW_VERSION_MAJOR
  fmt::println("GLFW is loaded");
#endif

  // log_load_config();

  // LogIniter::reg("system.debugger", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("system.console", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("system.fiber", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("net.tcp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("net.udp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("multimedia.rtp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("multimedia.rtsp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("multimedia.rtmp", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("multimedia.codec", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("multimedia.ffmpeg", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("multimedia.capture", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);
  // LogIniter::reg("app.screen.live", ".", LogIniterFlag::SYNC_FILE | LogIniterFlag::CONSOLE);

  std::vector<std::string> v = {
    "system.debugger",
    "system.console",
    "system.fiber",
    "net.tcp",
    "net.udp",
    "multimedia.rtp",
    "multimedia.rtsp",
    "multimedia.rtmp",
    "multimedia.codec",
    "multimedia.ffmpeg",
    "multimedia.capture",
    "app.screen.live"
  };
  for (const auto &x : v) {
    LogIniter::reg(x);
  }

  // log_export_config();
  // ffmpeg::reg_ffmpeg(ffmpeg::RegFlag::ALL);

  /*
  Thread main{"main"};
  main.dispatch([](){
    // test_alsa();
    start_screenlive();
  });
  */

  GET_LOGGER("root")->setLevel(LogLevel::LTRACE);
  GET_LOGGER("net")->setLevel(LogLevel::LTRACE);

  ILOG_TRACE(GET_LOGGER("root")) << "Hello, World!";

	EventLoop eventLoop;
	auto rtmp_server = RtmpServer::create(&eventLoop);
	rtmp_server->setChunkSize(60000);
	// rtmp_server->setGopCache(); // enable gop cache

	rtmp_server->setEventCallback([](std::string type, std::string stream_path) {
		printf("[Event] %s, stream path: %s\n\n", type.c_str(), stream_path.c_str());
	});

	if (!rtmp_server->start("127.0.0.1", 1935, 1024)) {
		printf("start rtmpServer error\n");
	}

  while(true) std::this_thread::sleep_for(100ms);

  rtmp_server->stop();

#ifdef __WIN__
  WSACleanup();
#endif
}
