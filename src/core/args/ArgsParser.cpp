
#include <ostream>
#include <vector>
#include <core/args/ArgsParser.h>
#include <core/util/StringUtil.h>
#include <core/util/logger/Logger.h>
#include <fmt/core.h>
#include "core/util/Util.h"

LY_NAMESPACE_BEGIN

void ArgsParser::parse(std::string_view argStr) {
  auto v = string_util::split(argStr, " ");
  this->parse(v);
}
void ArgsParser::parse(int argc, const char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    auto arg = std::string_view{argv[i]};
    LY_ASSERT(arg[0] == '-');
    auto mid = arg.find('=');
    LY_ASSERT(mid != std::string::npos);
    this->match(std::string{arg.substr(1, mid - 1)}) = arg.substr(mid + 1);
  }
}
void ArgsParser::parse(const std::vector<std::string> &argStrList) {
  for (auto &arg : argStrList) {
    auto mid = arg.find('=');
    LY_ASSERT(mid != std::string::npos);
    this->match(arg.substr(1, mid - 1)) = arg.substr(mid + 1);
  }
}

void ArgsParser::print(std::ostream &os) const {
  os << "Args:\n";
  for (auto& [key, op] : options_) {
    os << fmt::format(
        "\t-{}={:<20}"
            "\t{}\n",
            key,
            op.defaultValue(),
            (op.isRequired() ? "[Required]" : "[Selective]"),
            op.comment());
  }
  os << "Example: -key1=value1 -key2=value2 ...\n";
}

void ArgsParser::info(std::ostream &os) const {
  os << "Register args:\n";
  for (auto& [key, op] : options_) {
    os << fmt::format(
        "\t-{}={:<20}"
            "\t{}\n",
            key,
            (op.hasValue() ? op.value() : ""),
            op.comment());
  }
}
void ArgsParser::verbose(std::ostream &os) const {
  os << "Register args:\n";
  for (auto& [key, op] : options_) {
    os << fmt::format(
        "\t-{}={:<20}"
            "\t{}{}"
            "\t{}\n",
            key,
            (op.hasValue() ? op.value() : ""),
            (op.isRequired() ? "[Required]" : "[Selective]"),
            (op.hasValue() ? "√" : "x"),
            op.comment());
  }
}


ArgsParser& ArgsParser::add(const Option &option) {
  options_.emplace(option.key(), option);
  return *this;
}
ArgsParser& ArgsParser::add(Option &&option) {
  options_.emplace(option.key(), std::move(option));
  return *this;
}

ArgsParser& ArgsParser::operator<<(const Option &option) {
  return this->add(option);
}
ArgsParser& ArgsParser::operator<<(Option &&option) {
  return this->add(std::move(option));
}
bool ArgsParser::check() const {
  for (const auto &[_, op] : options_) {
    if (op.isRequired() && !op.hasValue()) {
      return false;
    }
  }
  return true;
}

LY_NAMESPACE_END
