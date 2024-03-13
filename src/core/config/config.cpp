#include <cstdint>
#include <fstream>
#include <stdexcept>

#include <core/config/config.h>
#include <core/util/logger/Logger.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN

void Config::toFile(const std::string &filename) const {
  std::ofstream out(filename);
  if (out.is_open()) {
    out << this->toYamlNode();
    out.flush();
  }
  else {
    throw std::invalid_argument("Config: There is no target file");
  }
}

Config Config::fromFile(const std::string &filename) {
  Config config;
  YAML::Node root = YAML::LoadFile(filename);
  // parse common
  {
    YAML::Node common = root["common"];
    // logger
    YAML::Node logger = common["logger"];
    if (logger.IsDefined()) LogIniter::loadYamlNode(logger);
    config.common.logger.max_file_line = logger["max_file_line"].as<uint64_t>();
    // buffer
    YAML::Node buffer = common["buffer"];
    config.common.buffer.max_buffer_size = buffer["max_buffer_size"].as<uint32_t>();
    config.common.buffer.max_bytes_per_read = buffer["max_bytes_per_read"].as<uint32_t>();
    // thread_pool
    YAML::Node thread_pool = common["thread_pool"];
    config.common.thread_pool.thread_num = thread_pool["thread_num"].as<uint32_t>();
    // fiber
    YAML::Node fiber = common["fiber"];
    config.common.fiber.max_buffer_size = fiber["max_buffer_size"].as<uint32_t>();
    config.common.fiber.max_fiber_capacity_per_scheduler = fiber["max_fiber_capacity_per_scheduler"].as<uint32_t>();
    // timer
    YAML::Node timer = common["timer"];
    config.common.timer.timeout_if_not_task_ms = timer["timeout_if_not_task_ms"].as<int>();
  }

  // parse net
  {
    YAML::Node net = root["net"];
    // common
    YAML::Node common = net["common"];
    config.net.common.max_upload_rate = common["max_upload_rate"].as<int>();
    config.net.common.max_download_rate = common["max_download_rate"].as<int>();
    config.net.common.send_timeout_ms = common["send_timeout_ms"].as<int>();
    config.net.common.recv_timeout_ms = common["recv_timeout_ms"].as<int>();
    config.net.common.timeout_while_adding_connection_ms = common["timeout_while_adding_connection_ms"].as<int>();
    config.net.common.timeout_while_removing_connections_ms = common["timeout_while_removing_connections_ms"].as<int>();
    config.net.common.handle_max_events_once = common["handle_max_events_once"].as<int>();
    config.net.common.max_sender_buffer_size = common["max_sender_buffer_size"].as<int>();
    config.net.common.max_receiver_buffer_size = common["max_receiver_buffer_size"].as<int>();
    // http
    YAML::Node http = net["http"];
    config.net.http.port = http["port"].as<uint16_t>();
    config.net.http.ssl_port = http["ssl_port"].as<uint16_t>();
    // rtsp
    YAML::Node rtsp = net["rtsp"];
    config.net.rtsp.port = rtsp["port"].as<uint16_t>();
    config.net.rtsp.ssl_port = rtsp["ssl_port"].as<uint16_t>();
    config.net.rtsp.connection_type = (net::rtsp::ConnectionType)rtsp["connection_type"].as<uint16_t>();
    config.net.rtsp.multicast_range.begin = rtsp["multicast_range"]["begin"].as<uint16_t>();
    config.net.rtsp.multicast_range.end = rtsp["multicast_range"]["end"].as<uint16_t>();
    // rtmp
    YAML::Node rtmp = net["rtmp"];
    config.net.rtmp.port = rtmp["port"].as<uint16_t>();
    config.net.rtmp.ssl_port = rtmp["ssl_port"].as<uint16_t>();
  }

  // parse multimedia
  {
    YAML::Node multimedia = root["multimedia"];
    // ffmpeg
    YAML::Node ffmpeg = multimedia["ffmpeg"];
    config.multimedia.ffmpeg.open_debug = ffmpeg["open_debug"].as<bool>();
    config.multimedia.ffmpeg.flags = ffmpeg["flags"].as<uint8_t>();
    ffmpeg::reg_ffmpeg(config.multimedia.ffmpeg.flags);
  }

  return config;
}


YAML::Node Config::toYamlNode() const {
  YAML::Node root;
  // parse common
  {
    YAML::Node common = root["common"];
    /** logger **/
    YAML::Node logger = common["logger"];
    logger["max_file_line"] = g_config.common.logger.max_file_line;
    /** buffer **/
    YAML::Node buffer = common["buffer"];
    buffer["max_buffer_size"] = g_config.common.buffer.max_buffer_size;
    buffer["max_bytes_per_read"] = g_config.common.buffer.max_bytes_per_read;
    /** thread pool **/
    YAML::Node thread_pool = common["thread_pool"];
    thread_pool["thread_num"] = g_config.common.thread_pool.thread_num;
    // fiber
    YAML::Node fiber = common["fiber"];
    fiber["max_buffer_size"] = g_config.common.fiber.max_buffer_size;
    fiber["max_fiber_capacity_per_scheduler"] = g_config.common.fiber.max_fiber_capacity_per_scheduler;
    // timer
    YAML::Node timer = common["timer"];
    timer["timeout_if_not_task_ms"] = g_config.common.timer.timeout_if_not_task_ms;
  }

  // parse net
  {
    YAML::Node net = root["net"];
    // common
    YAML::Node common = net["common"];
    common["max_upload_rate"] = g_config.net.common.max_upload_rate;
    common["max_download_rate"] = g_config.net.common.max_download_rate;
    common["send_timeout_ms"] = g_config.net.common.send_timeout_ms;
    common["recv_timeout_ms"] = g_config.net.common.recv_timeout_ms;
    common["timeout_while_adding_connection_ms"] = g_config.net.common.timeout_while_adding_connection_ms;
    common["timeout_while_removing_connections_ms"] = g_config.net.common.timeout_while_removing_connections_ms;
    common["handle_max_events_once"] = g_config.net.common.handle_max_events_once;
    // http
    YAML::Node http = net["http"];
    http["port"] = g_config.net.http.port;
    http["ssl_port"] = g_config.net.http.ssl_port;
    // rtsp
    YAML::Node rtsp = net["rtsp"];
    rtsp["port"] = g_config.net.rtsp.port;
    rtsp["ssl_port"] = g_config.net.rtsp.ssl_port;
    rtsp["connection_type"] = (uint16_t)g_config.net.rtsp.connection_type;
    rtsp["multicast_range"]["begin"] = g_config.net.rtsp.multicast_range.begin;
    rtsp["multicast_range"]["end"] = g_config.net.rtsp.multicast_range.end;
    // rtmp
    YAML::Node rtmp = net["rtmp"];
    rtmp["port"] = g_config.net.rtmp.port;
    rtmp["ssl_port"] = g_config.net.rtmp.ssl_port;
  }

  // parse multimedia
  {
    YAML::Node multimedia = root["multimedia"];
    // ffmpeg
    YAML::Node ffmpeg = multimedia["ffmpeg"];
    ffmpeg["open_debug"] = g_config.multimedia.ffmpeg.open_debug;
    ffmpeg["flags"] = g_config.multimedia.ffmpeg.flags;
  }

  return root;
}

LY_NAMESPACE_END
