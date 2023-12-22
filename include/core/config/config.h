#pragma once

#include <thread>
#include <string_view>
#include <string>
#include <sstream>

#include <yaml-cpp/yaml.h>

#include "marcos.h"

enum class ConfigType 
{
  T_YAML,
  T_TOML,
};

struct Config {
  ConfigType use_yaml_or_toml = ConfigType::T_TOML;

  struct 
  {
    /** common **/
    int thread_num = std::thread::hardware_concurrency();
    int max_block_size = 1024;

  } common;
  
  struct 
  {
    /** net **/
    int max_upload_rate = -1;
    int max_download_rate = -1;
    int send_timeout_ms = 2000;
    int recv_timeout_ms = 2000;
    int connection_timeout_ms = 15000;
  } net;

  struct 
  {
    /** buffer **/
    int buffer_size = 4096;

  } buffer;

  std::string toString() const;
  void toFile(std::string_view filename) const;

private:
  std::string toYamlString() const;
  std::string toTomlString() const;
  std::string toIniString() const;
};

inline std::string Config::toString() const 
{
  if (use_yaml_or_toml == ConfigType::T_TOML) {
    return toTomlString();
  }
  else if (use_yaml_or_toml == ConfigType::T_YAML) {
    return toYamlString();
  }

  UNREACHABLE();
}
inline void Config::toFile(std::string_view filename) const 
{
  std::ofstream ofs(filename.data());
  if (ofs.is_open()) {
    ofs << toString();
    ofs.flush();
  }
  else {
    throw std::runtime_error("There is no target file");
  }
}

inline std::string Config::toYamlString() const
{
}

inline std::string Config::toTomlString() const
{
}


static Config g_config = {};
