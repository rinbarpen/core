#include <sstream>

#include <core/util/Util.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
static auto g_system_logger = GET_LOGGER("system");

std::string backtrace(std::string_view prefix, int stack_capacity, int skip)
{
  LY_ASSERT2(stack_capacity > 0 && skip > 0, "Backtrace");

  std::ostringstream bt_ss;
  bt_ss << prefix;

  auto bt = (void**)::malloc(sizeof(void*) * stack_capacity);
  int depth = ::backtrace(bt, stack_capacity);
  if (depth > 0) {
    char **s = ::backtrace_symbols(bt, depth);
    if (nullptr == s) {
      ILOG_ERROR(g_system_logger) << "Cannot fetch the symbols of backtrace";
      ::free(bt);
      return "";
    }

    for (int i = skip; i < depth; ++i) {
      bt_ss << "===[" << i << "]: " << s[i];
    }
    ::free(s);
  }

  ::free(bt);
  return bt_ss.str();
}
LY_NAMESPACE_END
