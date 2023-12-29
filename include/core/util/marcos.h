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

#if (defined(WIN32) || defined(_WIN32)) && !defined(__WIN__)
# define __WIN__
#elif (defined(__linux) || defined(__linux__)) && !defined(__LINUX__)
# define __LINUX__
#endif

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <type_traits>


#define LY_AUTHOR  LpoutYoumu
#define LY_VERSION 01L

#define LY_ASSERT(x) assert(x)
#define LY_CHECK(x, err_msg) static_assert(x, err_msg)

#define NAMESPACE_BEGIN(x) \
  namespace x             \
  {
#define NAMESPACE_END(x) \
  }

#define EMPTY_NAMESPACE_BEGIN \
  namespace \
  {
#define EMPTY_NAMESPACE_END  \
  }

#define LY_NAMESPACE_BEGIN \
  namespace ly            \
  {
#define LY_NAMESPACE_END \
  }


LY_NAMESPACE_BEGIN

class UnreachableException : public ::std::runtime_error {
public:
  UnreachableException(const char *msg) : ::std::runtime_error(msg) {}
};

#define __UNREACHABLE_THROW()                                                    \
  do {                                                                         \
    char __s[100];                                                             \
    ::snprintf(__s, 100, "Unreachable has been called by %s in %s:%d",         \
               __FUNCTION__, __FILE__, __LINE__);                              \
    throw ::ly::UnreachableException(__s);                                         \
  } while (0)

#define __UNREACHABLE_NOTHROW()                                                  \
  do {                                                                         \
    ::fprintf(stderr, "Unreachable has been called by %s in %s:%d",            \
              __FUNCTION__, __FILE__, __LINE__);                               \
  } while (0)

#define LY_UNREACHABLE() __UNREACHABLE_THROW()


#define LY_NONCOPYABLE(CLS)                                                      \
  CLS(const CLS &) = delete;                                                   \
  CLS &operator=(const CLS &) = delete

#define LY_NONMOVEABLE(CLS)                                                      \
  CLS(CLS &&) = delete;                                                        \
  CLS &operator=(CLS &&) = delete


#define LY_UNUSED [[maybe_unused]]
#define LY_NODISCARD [[nodiscard]]

#define SHARED_PTR_USING(CLS, ALIAS)    using ALIAS = std::shared_ptr<CLS>
#define UNIQUE_PTR_USING(CLS, ALIAS)    using ALIAS = std::unique_ptr<CLS>
#define WEAK_PTR_USING(CLS, ALIAS)      using ALIAS = std::weak_ptr<CLS>
#define SHARED_PTR_TYPEDEF(CLS, ALIAS)  typedef std::shared_ptr<CLS> ALIAS
#define UNIQUE_PTR_TYPEDEF(CLS, ALIAS)  typedef std::unique_ptr<CLS> ALIAS
#define WEAK_PTR_TYPEDEF(CLS, ALIAS)    typedef std::weak_ptr<CLS>   ALIAS

#define MAKE_SHARED(CLS)  \
  template <typename... Args> \
  LY_NODISCARD static inline std::shared_ptr<CLS> make_shared(Args &&...args) \
  { \
    return std::make_shared<CLS>(std::forward<Args>(args)...); \
  }

#define SHARED_REG(CLS) \
  SHARED_PTR_USING(CLS, ptr); \
  MAKE_SHARED(CLS)

#define MAKE_UNIQUE(CLS)                                                       \
  template <typename... Args>                                                  \
  LY_NODISCARD static inline std::unique_ptr<CLS> make_unique(Args &&...args) \
  {                                                                            \
    return std::make_unique<CLS>(std::forward<Args>(args)...);                 \
  }

#define UNIQUE_REG(CLS)       \
  UNIQUE_PTR_USING(CLS, uptr); \
  MAKE_UNIQUE(CLS)

#ifdef __CXX17
#define LY_UNWARP(TYPE, WRAPPER, ...) \
  auto TYPE[__VA_ARGS__] = WRAPPER
#endif

template <typename T, typename... U>
struct is_any_of : public std::disjunction<std::is_same<T, U>...> {
};
template <typename T, typename... U>
using is_any_of_t = typename is_any_of<T, U...>::type;
template <typename T, typename... U>
inline constexpr bool is_any_of_v = is_any_of<T, U...>::value;

template <typename T, typename... U>
struct is_same_all : public std::is_same<std::is_same<T, U>...> {
};
template <typename T, typename... U>
using is_same_all_t = typename is_same_all<T, U...>::type;
template <typename T, typename... U>
inline constexpr bool is_same_all_v = is_same_all<T, U...>::value;

template <typename T>
struct is_qstring_like {
private:
  template <typename U>
  static auto check(int)
    -> decltype(std::declval<U>().fromStdString(std::string{}),
      std::declval<U>().toStdString(), std::true_type{});

  template <typename>
  static auto check(...) -> std::false_type;

public:
  static constexpr bool value = decltype(check<T>(0))::value;
};
template <class QStringLike>
inline constexpr bool is_qstring_like_v = is_qstring_like<QStringLike>::value;

#ifdef __CXX20
template <typename T>
concept QStringLike = requires(T a) {
  { a.toStdString() } -> std::convertible_to<std::string>;
  { T::fromStdString(std::string{}) } -> std::same_as<void>;
};
#endif

LY_NAMESPACE_END

/*********************************************************************************/
//                                cxx dependencies
/*********************************************************************************/
