#pragma once

#include <core/util/marcos.h>

class FSLogicException : public std::logic_error 
{
public:
  FSLogicException(const char *msg)
    : std::logic_error(msg)
  {}
};


class FSException : public std::runtime_error
{
public:
  FSException(const char *msg)
    : std::runtime_error(msg)
  {}

};
