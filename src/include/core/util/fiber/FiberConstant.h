#pragma once

#include <core/util/marcos.h>
#ifdef __LINUX__
#include <ucontext.h>
#endif

LY_NAMESPACE_BEGIN

inline static constexpr int kDefaultMaxFiberBufferSize = 1024 * 1024;
inline static constexpr int kDefaultMaxFiberPerScheduler = 100;

using FiberId = uint32_t;


LY_NAMESPACE_END
