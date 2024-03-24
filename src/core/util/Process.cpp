#include <core/util/Process.h>

LY_NAMESPACE_BEGIN
Process::Process(Process::ProcFn fn)
  : fn_(fn)
{}
Process::~Process()
{
  kill(pid_, SIGKILL);
  pid_ = -1;
}
void Process::run()
{
  pid_t pid = fork();
  if (pid < 0) {
    return ;
  }
  else if (pid == 0) {
    pid_ = pid;
    try {
      fn_();
    } catch(...) {
    }
  }
}
void Process::terminal()
{
  sendSignal(SIGKILL);
}
void Process::hup()
{
  sendSignal(SIGHUP);
}
void Process::sendSignal(int sig)
{
  kill(pid_, sig);
}

LY_NAMESPACE_END
