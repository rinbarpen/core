/**
 * @file marcos.h
 * @author LpoutYoumu (LpoutYoumu@gmail.com)
 * @brief This file includes some macros we need
 * @version 0.1
 * @date 2024-02-11
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

/*********************************************************************************/
//                            confirm cxx version
/*********************************************************************************/
#if defined(__clang__) || defined(__GNUC__)
# define __CXX_STANDARD __cplusplus
#elif defined(_MSC_VER)
# define __CXX_STANDARD _MSVC_LANG
#endif

#if __CXX_STANDARD >= 201703L
# define __CXX17
#elif __CXX_STANDARD >= 201402L
# define __CXX14
#elif __CXX_STANDARD >= 201103L
# define __CXX11
#endif

/*********************************************************************************/
//                            which platform?
/*********************************************************************************/
#if (defined(WIN32) || defined(_WIN32)) && !defined(__WIN__)
# define __WIN__
#elif (defined(__linux) || defined(__linux__)) && !defined(__LINUX__)
# define __LINUX__
#endif

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>



#define LY_AUTHOR    LpoutYoumu
#define LY_VERSION   0.1

#define LY_ASSERT(x) assert(x)
#define LY_ASSERT2(x, detail)                             \
  do {                                                    \
    if (!(x))                                             \
    {                                                     \
      static auto system_logger = GET_LOGGER("system"); \
      ILOG_FATAL(system_logger) << (detail);            \
      std::abort();                                       \
    }                                                     \
  } while (0)
#define LY_ASSERT3(x, detail)           \
  do {                                  \
    if (!(x))                           \
    {                                   \
      throw std::runtime_error(detail); \
    }                                   \
  } while (0)
#define LY_CHECK(x, err_msg) static_assert(x, err_msg)

#define NAMESPACE_BEGIN(x) \
  namespace x              \
  {
#define NAMESPACE_END(x) }  // namespace x

#define EMPTY_NAMESPACE_BEGIN \
  namespace                   \
  {
#define EMPTY_NAMESPACE_END }

#define LY_NAMESPACE_BEGIN  NAMESPACE_BEGIN(ly)
#define LY_NAMESPACE_END    NAMESPACE_END(ly)


LY_NAMESPACE_BEGIN
namespace detail
{
class UnreachableException : public ::std::runtime_error
{
public:
  UnreachableException(const char *msg) : ::std::runtime_error(msg) {}
};
}  // namespace detail

#define __UNREACHABLE_THROW()                                          \
  do {                                                                 \
    char __s[100];                                                     \
    ::snprintf(__s, 100, "Unreachable has been called by %s in %s:%d", \
      __FUNCTION__, __FILE__, __LINE__);                               \
    throw ::ly::detail::UnreachableException(__s);                     \
  } while (0)

#define __UNREACHABLE_NOTHROW()                                     \
  do {                                                              \
    ::fprintf(stderr, "Unreachable has been called by %s in %s:%d", \
      __FUNCTION__, __FILE__, __LINE__);                            \
  } while (0)

#define LY_UNREACHABLE() __UNREACHABLE_THROW()


#define LY_NONCOPYABLE(CLS)  \
  CLS(const CLS &) = delete; \
  CLS &operator=(const CLS &) = delete

#define LY_NONMOVEABLE(CLS) \
  CLS(CLS &&) = delete;     \
  CLS &operator=(CLS &&) = delete


#define LY_UNUSED     [[maybe_unused]]
#define LY_NODISCARD  [[nodiscard]]
#define LY_DEPRECATED [[deprecated]]
#ifdef __CXX20
# define LY_LIKELY   [[likely]]
# define LY_UNLIKELY [[unlikely]]
#elif defined(__GNUC__) || defined(__llvm__)
# define LY_LIKELY(x)   __builtin_expect(!!(x), 1)
# define LY_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define LY_LIKELY(x)   (x)
# define LY_UNLIKELY(x) (x)
#endif

#define SHARED_PTR_USING(CLS, ALIAS)   using ALIAS = std::shared_ptr<CLS>
#define UNIQUE_PTR_USING(CLS, ALIAS)   using ALIAS = std::unique_ptr<CLS>
#define WEAK_PTR_USING(CLS, ALIAS)     using ALIAS = std::weak_ptr<CLS>
#define SHARED_PTR_TYPEDEF(CLS, ALIAS) typedef std::shared_ptr<CLS> ALIAS
#define UNIQUE_PTR_TYPEDEF(CLS, ALIAS) typedef std::unique_ptr<CLS> ALIAS
#define WEAK_PTR_TYPEDEF(CLS, ALIAS)   typedef std::weak_ptr<CLS> ALIAS

#define MAKE_SHARED(CLS)                                       \
  template <typename... Args>                                  \
  LY_NODISCARD static inline std::shared_ptr<CLS> make_shared( \
    Args &&...args) {                                          \
    return std::make_shared<CLS>(std::forward<Args>(args)...); \
  }

#define SHARED_REG(CLS)       \
  SHARED_PTR_USING(CLS, ptr); \
  MAKE_SHARED(CLS)

#define MAKE_UNIQUE(CLS)                                       \
  template <typename... Args>                                  \
  LY_NODISCARD static inline std::unique_ptr<CLS> make_unique( \
    Args &&...args) {                                          \
    return std::make_unique<CLS>(std::forward<Args>(args)...); \
  }

#define UNIQUE_REG(CLS)        \
  UNIQUE_PTR_USING(CLS, uptr); \
  MAKE_UNIQUE(CLS)

#define LY_UNWARP(TYPE, WRAPPER, ...) auto TYPE[__VA_ARGS__] = WRAPPER

#define LY_GLOBAL_BLOCK(BODY) \
  namespace global            \
  {                           \
  BODY                        \
  }

LY_NAMESPACE_END

/*********************************************************************************/
//                                cxx dependencies
/*********************************************************************************/
