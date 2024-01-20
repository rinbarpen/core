#pragma once

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

    } https;
    struct
    {

    } ftp;
    struct
    {

    } sftp;
    struct
    {

    } websocket;
    struct
    {

    } hls;
    struct
    {

    } rtsp;
    struct
    {

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
