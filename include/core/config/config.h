#pragma once

#include <thread>
#include <string_view>
#include <string>
#include <sstream>

#include <yaml-cpp/yaml.h>

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

struct Config {
  struct 
  {
    /** common **/
    struct
    {
      int thread_num = std::thread::hardware_concurrency();

    } threadpool;
    struct
    {
      
    } logger;
    
    struct
    {
      /** buffer **/
      int max_buffer_size = 1024;           // 
      
    } buffer;
  } common;
  
  struct 
  {
    /** net **/
    int max_upload_rate = -1;
    int max_download_rate = -1;
    int send_timeout_ms = 2000;
    int recv_timeout_ms = 2000;
    int connection_timeout_ms = 15000;

    int max_sender_buffer_size =
      1024;  // use ${max_buffer_size} instead if exceeding ${max_buffer_size}
    int max_receiver_buffer_size =
      1024;  // use ${max_buffer_size} instead if exceeding ${max_buffer_size}
  } net;


  std::string toString() const;
  void toFile(const std::string &filename) const;

  static Config fromFile(const std::string &filename);

private:
  std::string toYamlString() const;
};


static Config g_config = {};

LY_NAMESPACE_END
