#pragma once

#include <core/util/marcos.h>
#include <cstring>
#include <stdexcept>

namespace ly::ffmpeg
{

class RuntimeException : std::runtime_error
{
public:
  RuntimeException(const char *msg)
    : std::runtime_error(msg)
  {}
};
class InitializationException : std::runtime_error
{
public:
  InitializationException(const char *msg)
    : std::runtime_error(msg)
  {}
};
class LogicalException : std::logic_error
{
public:
  LogicalException(const char *msg)
    : std::logic_error(msg)
  {}
};

}
