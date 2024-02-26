#pragma once

#include <core/util/marcos.h>
#include <core/util/Traits.h>
#include <string_view>

LY_NAMESPACE_BEGIN

class Translator
{
public:
  // TODO: Test me!
  template <class QStringLike, typename Fn, typename... Args>
  static auto qstrCallConstRef(QStringLike &&qstr, Fn &&fn, Args &&... args)
    -> std::invoke_result_t<Fn, const std::string&, Args...> {
    LY_CHECK(is_qstring_like_v<QStringLike>, "This Function is used by QStringLike");
    return std::forward<Fn>(fn)(qstr.toStdString(), std::forward<Args>(args)...);
  }
  template <class QStringLike, typename Fn, typename... Args>
  static auto qstrCallView(QStringLike &&qstr, Fn &&fn, Args &&... args)
    -> std::invoke_result_t<Fn, std::string_view, Args...> {
    LY_CHECK(is_qstring_like_v<QStringLike>, "This Function is used by QStringLike");
    return std::forward<Fn>(fn)(std::string_view{qstr.toStdString()}, std::forward<Args>(args)...);
  }
};

LY_NAMESPACE_END
