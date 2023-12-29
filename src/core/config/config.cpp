#include <core/config/config.h>

#include <fstream>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN

std::string Config::toString() const
{
  return toYamlString();  
}
void Config::toFile(const std::string &filename) const
{
  std::ofstream out(filename);
  if (out.is_open())
  {
    out << this->toString();
    out.flush();
  }
  else
  {
    throw std::runtime_error("There is no target file");
  }
}

Config Config::fromFile(const std::string &filename) 
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

  // parse av
  {

  }
  return config;
}

LY_NAMESPACE_END
