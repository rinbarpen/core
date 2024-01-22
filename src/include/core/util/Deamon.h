#pragma once

#include <string>

#include <core/util/marcos.h>


LY_NAMESPACE_BEGIN

class Deamon
{
public:
  Deamon(const char *path);
  ~Deamon();

  void run(char *const* cmd);
  int exitCode() const { return exit_code_; }

private:
  std::string exec_;
  int exit_code_{0};
  pid_t pid_;
};

LY_NAMESPACE_END
