#pragma once
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

// no performance loss
// more performance than std::function
template <typename Fn, typename... Args>
struct FunctionWrapper
{
  Fn fn;
  std::tuple<Args...> args;

  FunctionWrapper(Fn &&_fn, Args &&..._args)
    : fn(_fn), args(std::make_tuple(_args...))
  {}

  auto operator()() const -> decltype(auto) { return std::apply(fn, args); }
};

LY_NAMESPACE_END
