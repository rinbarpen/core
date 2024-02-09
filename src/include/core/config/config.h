#pragma once

#include <cstdint>
#include <thread>
#include <string>

#include <yaml-cpp/yaml.h>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

struct Config {
  struct
  {
    /** common **/
    struct
    {
      uint64_t max_file_line = 100000;
    } logger;
    struct
    {
      /** buffer **/
      uint32_t max_buffer_size = 4096;           //
      uint32_t max_bytes_per_read = 4096;
    } buffer;
    struct
    {
      int thread_num = std::thread::hardware_concurrency();

    } threadpool;
  } common;

  struct
  {
    /** net **/
    struct
    {
      int max_upload_rate = -1;
      int max_download_rate = -1;
      int send_timeout_ms = 2000;
      int recv_timeout_ms = 2000;
      int connection_timeout_ms = 15000;

      int max_sender_buffer_size =
        1024;  // use ${max_buffer_size} instead if exceeding ${max_buffer_size}
      int max_receiver_buffer_size =
        1024;  // use ${max_buffer_size} instead if exceeding ${max_buffer_size}
    } util;

    struct
    {

    } quic;

    struct
    {
      uint16_t port = 80;
      uint16_t ssl_port = 443;
    } https;
    struct
    {
      uint16_t port = 22;
    } ftp;
    struct
    {
      uint16_t port = 23;
    } sftp;
    struct
    {
      uint16_t port = 80;
      uint16_t ssl_port = 443;
    } websocket;
    struct
    {
      uint16_t port = 80;
      uint16_t ssl_port = 443;
    } hls;
    struct
    {
      uint16_t port = 443;
      uint16_t ssl_port = 1443;
    } rtsp;
    struct
    {
      uint16_t port = 1935;
      uint16_t ssl_port = 19350;
    } rtmp;
    struct
    {
    } webrtc;
  } net;

  struct
  {
    /** multimedia **/
    struct
    {
      bool open_debug = true;
      uint8_t flag = 0xFF;  // open all ffmpeg modules
    } ffmpeg;
    struct
    {

    } opengl;
    struct
    {

    } d3d9;
    struct
    {

    } nvidia;
  } multimedia;


  void toFile(const std::string &filename) const;
  static auto fromFile(const std::string &filename) -> Config;

private:
  auto toYamlNode() const -> YAML::Node;
};

static Config g_config = {};

LY_NAMESPACE_END
