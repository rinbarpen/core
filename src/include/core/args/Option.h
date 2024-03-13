#pragma once

#include <stdexcept>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
class OptionInitializedException : public std::runtime_error
{
public:
  OptionInitializedException() : std::runtime_error("Not be initialized") {}
};

class OptionValue
{
public:
  OptionValue() {}
  OptionValue(const std::string &value)
    : on_(true), value_(value), default_value_(value)
  {}

  std::string &value() {
    on_ = true;
    return value_;
  }
  std::string value() const {
    if (on_) return value_;
    throw OptionInitializedException();
  }
  std::string defaultValue() const { return default_value_; }

  bool hasValue() const { return on_; }

private:
  bool on_{false};
  const std::string default_value_;
  std::string value_;
};
class Option
{
public:
  SHARED_PTR_USING(Option, ptr);

  Option(const std::string &key, OptionValue defaultValue, const std::string &comment, bool required = true)
    : key_(key), value_(defaultValue), comment_(comment), required_(required)
  {}

  std::string &value() { return value_.value(); }
  std::string value() const { return value_.value(); }
  std::string defaultValue() const { return value_.defaultValue(); }

  bool hasValue() const { return value_.hasValue(); }

  std::string key() const { return key_; }
  std::string comment() const { return comment_; }
  bool isRequired() const { return required_; }

private:
  const std::string key_;
  OptionValue value_;
  std::string comment_;
  bool required_{true};
};
LY_NAMESPACE_END
