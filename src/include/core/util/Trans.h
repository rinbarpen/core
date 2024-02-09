#pragma once

#include <core/util/marcos.h>
#include <core/util/Traits.h>
#include <string_view>
#include <type_traits>

LY_NAMESPACE_BEGIN

class Translator
{
public:
  // TODO: Test me!
  template <class QStringLike, typename Fn, typename... Args>
  static auto qstrCall(QStringLike &&qstr, Fn &&fn, Args &&... args)
    -> std::invoke_result_t<Fn, std::string_view, Args...> {
    LY_CHECK(is_qstring_like_v<QStringLike>, "This Function is used by QStringLike");
    return fn(qstr.toStdString(), std::forward<Args>(args)...);
  }
};

LY_NAMESPACE_END
