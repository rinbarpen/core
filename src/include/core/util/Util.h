#pragma once
#include <sstream>
#include <utility>

#include <core/util/marcos.h>
#include <core/util/OSUtil.h>
#include <core/util/StringUtil.h>
#include <core/util/Traits.h>
#include <core/util/FunctionWrapper.h>

#ifdef __LINUX__
# include <execinfo.h>
#endif

LY_NAMESPACE_BEGIN
static std::string backtrace(
  std::string_view prefix, int stack_capacity = 20, int skip = 0)
{
  assert(stack_capacity > 0 && skip > 0);

  std::ostringstream bt_ss;
  bt_ss << prefix;

  auto bt = (void**)::malloc(sizeof(void*) * stack_capacity);
  int depth = ::backtrace(bt, stack_capacity);
  if (depth > 0) {
    char **s = ::backtrace_symbols(bt, depth);
    if (nullptr == s) {
      ::free(bt);
      return "Cannot fetch the symbols of backtrace";
    }

    for (int i = skip; i < depth; ++i) {
      bt_ss << "===[" << i << "]: " << s[i];
    }
    ::free(s);
  }

  ::free(bt);
  return bt_ss.str();
}
template <typename... Args>
static void do_nothing(Args &&...args) {}
template <typename Fn, typename... Args>
static void loop(Fn &&fn, Args&&... args)
{
  for (;;)
    std::forward(fn)(std::forward(args)...);
}
template <typename Fn, typename... Args>
static void loop(FunctionWrapper<Fn, Args...> fn)
{
  for (;;) fn();
}

LY_NAMESPACE_END

#undef LY_ASSERT
#define LY_ASSERT(x)                                       \
  do {                                                     \
    if (!(x)) {                                            \
      static auto system_logger = GET_LOGGER("system");    \
      ILOG_FATAL(system_logger) << ::ly::backtrace("   "); \
      assert(x);                                           \
    }                                                      \
  } while (0)

#undef LY_ASSERT2
#define LY_ASSERT2(x, detail)                                                  \
  do {                                                                         \
    if (!(x)) {                                                                \
      static auto system_logger = GET_LOGGER("system");                        \
      ILOG_FATAL(system_logger) << (detail) << "\n" << ::ly::backtrace("   "); \
      std::abort();                                                            \
    }                                                                          \
  } while (0)
