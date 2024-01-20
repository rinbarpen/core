#include <core/util/Deamon.h>
#include <unistd.h>


LY_NAMESPACE_BEGIN
Deamon::Deamon(const char *path)
  : exec_(path)
{
  pid_ = getpid();
}
Deamon::~Deamon() { }

void Deamon::run(char *const* cmd)
{
  ::execvp(exec_.c_str(), cmd);
}

LY_NAMESPACE_END
