#pragma once

#include <functional>
#include <core/util/marcos.h>
#include <core/util/Traits.h>

#ifdef __LINUX__
# include <unistd.h>
# include <signal.h>
#endif

LY_NAMESPACE_BEGIN
class Process
{
public:
  using ProcFn = std::function<void()>;

  Process(ProcFn fn);
  ~Process();

  void run();
  void terminal();
  void hup();
  void sendSignal(int sig);

  pid_t pid() const { return pid_; }

private:
  pid_t pid_{-1};
  ProcFn fn_;
};
LY_NAMESPACE_END
