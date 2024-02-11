#include <fstream>
#include <sstream>
#include <string_view>

#include <core/util/logger/Logger.h>
#include <core/config/config.h>

LY_NAMESPACE_BEGIN

void Config::toFile(const std::string &filename) const
{
  std::ofstream out(filename);
  if (out.is_open())
  {
    out << this->toYamlNode();
    out.flush();
  }
  else
  {
    throw std::runtime_error("Config: There is no target file");
  }
}

auto Config::fromFile(const std::string &filename) -> Config
{
  Config config;
  YAML::Node root = YAML::LoadFile(filename);
  // parse common
  {
    YAML::Node common = root["common"];
    YAML::Node logger = common["logger"];

    if (logger.IsDefined())
      LogIniter::loadYamlNode(logger);

    YAML::Node buffer = common["buffer"];

    YAML::Node thread_pool = common["thread_pool"];
    config.common.threadpool.thread_num = thread_pool["thread_num"].as<int>();


  }

  // parse net
  {

  }

  // parse multimedia
  {

  }
  return config;
}


auto Config::toYamlNode() const -> YAML::Node
{
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
    /** threadpool **/
    YAML::Node threadpool = common["threadpool"];
    threadpool["thread_num"] = g_config.common.threadpool.thread_num;
  }

  // parse net
  {
    YAML::Node net = root["net"];
    YAML::Node logger = net["rtp"];
  }

  // parse multimedia
  {

  }

  return root;
}

LY_NAMESPACE_END
