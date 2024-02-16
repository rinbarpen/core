#pragma once

#include <string>
#include <core/util/marcos.h>
#if defined(__WIN__)
# include <Windows.h>
# include <process.h>
#elif defined(__LINUX__)
# include <unistd.h>
#endif

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
#if defined(__LINUX__)
  pid_t pid_;
#elif defined(__WIN__)
  DWORD pid_;
#endif
};

LY_NAMESPACE_END
