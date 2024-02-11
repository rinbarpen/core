#pragma once

#include <core/util/marcos.h>
#include <core/util/StringUtil.h>
#include <core/util/OSUtil.h>
#include <core/util/Traits.h>

#ifdef __LINUX__
#include <execinfo.h>
#endif

LY_NAMESPACE_BEGIN
static std::string backtrace(std::string_view prefix, int stack_capacity = 10, int skip = 0);

template <typename... Args>
static void do_nothing(Args &&... args) {}

LY_NAMESPACE_END
