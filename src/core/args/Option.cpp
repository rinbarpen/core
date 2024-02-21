#include <core/args/Option.h>

LY_NAMESPACE_BEGIN
Option::Option(const std::string &key, OptionValue defaultValue, const std::string &errorTip, const std::string &comment, bool required)
  : key_(key), value_(defaultValue), error_tip_(errorTip), comment_(comment), required_(required)
{

}
LY_NAMESPACE_END
