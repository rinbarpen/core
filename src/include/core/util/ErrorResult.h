#pragma once

#include "core/util/marcos.h"
#include <unordered_map>
LY_NAMESPACE_BEGIN

// TODO: For future. Union the error handle
class ErrorResult
{
public:

private:
  std::string name_;
  std::unordered_map<int, std::string> error_map_;
};

LY_NAMESPACE_END
