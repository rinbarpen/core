#pragma once

#include <core/util/marcos.h>

#include <type_traits>

LY_NAMESPACE_BEGIN

class Translator 
{
public:
  template <class QStringLike, typename Fn, typename... Args>
  static auto qstrCall(QStringLike qstr, Fn &&fn, Args &&...args)
    -> std::invoke_result_t<Fn, Args...>
  {
    LY_CHECK(decltype(std::remove_cv_t<QStringLike>::fromStdString(std::string{}),
        std::declval<std::remove_cv_t<QStringLike>>().toStdString())
        >, 
               "This Function is used by QStringLike");
    return fn(qstr.toStdString(), std::forward<Args>(args)...);
  }
};

LY_NAMESPACE_END
