#pragma once

#include <cstdint>
#include <thread>
#include <string>

#include <core/util/marcos.h>

#include <yaml-cpp/yaml.h>

LY_NAMESPACE_BEGIN

struct Config {
  struct common
  {
    /** common **/
    struct logger
    {
      uint64_t max_file_line = 100000;
      // const char *default_format_pattern =
      //   "$DATETIME{%Y-%m-%d %H:%M:%S}"
      //   "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
      //   "$CHAR:\t$FILENAME$CHAR::$LINE"
      //   "$CHAR:\t$FUNCTION_NAME"
      //   "$CHAR:\t$MESSAGE$CHAR:\n";

    } logger;
    struct buffer
    {
      /** buffer **/
      uint32_t max_buffer_size = 4096;           //
      uint32_t max_bytes_per_read = 4096;
    } buffer;
    struct threadpool
    {
      uint32_t thread_num = std::thread::hardware_concurrency();
    } thread_pool;
    struct fiber
    {
      uint32_t max_buffer_size = 1024 * 128;
      uint32_t max_fiber_capacity_per_scheduler = 100;
    } fiber;
    struct timer
    {
      int timeout_if_not_task_ms = 100;
    } timer;
  } common;

  struct net
  {
    /** net **/
    struct common
    {
      int max_upload_rate = -1;
      int max_download_rate = -1;
      int send_timeout_ms = 100;
      int recv_timeout_ms = 100;
      int timeout_while_adding_connection_ms = 15000;
      int timeout_while_removing_connections_ms = 100;
      int handle_max_events_once = 1024;

      int max_sender_buffer_size = 1024 * 1024;
      int max_receiver_buffer_size = 1024 * 1024;
    } common;

    struct quic
    {

    } quic;

    struct http
    {
      uint16_t port = 80;
      uint16_t ssl_port = 443;
    } http;
    struct rtsp
    {
      enum ConnectionType {
        AUTO = 0xFF,
        TCP = 0x01,
        UDP = 0x02,
        MULTICAST = 0x04,
      };

      uint16_t port = 554;
      uint16_t ssl_port = 1554;
      ConnectionType connection_type = ConnectionType::AUTO;

      struct multicast_range {
        uint16_t begin = 15000;
        uint16_t end = 15100;
      } multicast_range;
    } rtsp;
    struct rtmp
    {
      uint16_t port = 1935;
      uint16_t ssl_port = 19350;
    } rtmp;
  } net;

  struct multimedia
  {
    /** multimedia **/
    struct ffmpeg
    {
      bool open_debug = true;
      uint8_t flags = 0xFF;  // open all ffmpeg modules
    } ffmpeg;
  } multimedia;


  void toFile(const std::string &filename) const;
  static Config fromFile(const std::string &filename);

  static Config& instance()
  {
    static Config config;
    return config;
  }

private:
  YAML::Node toYamlNode() const;
};

static Config &g_config = Config::instance();

#define LY_CONFIG_GET(FIELD) \
  Config::instance().FIELD
#define LY_CONFIG_SET(FIELD, VALUE) \
  do { Config::instance().FIELD = VALUE } while(0)

LY_NAMESPACE_END
