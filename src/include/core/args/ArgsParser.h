#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <core/args/Option.h>

LY_NAMESPACE_BEGIN
class ArgsParser
{
public:
  ArgsParser() = default;
  ~ArgsParser() = default;

  void parse(std::string_view argStr);
  void parse(int argc, const char *argv[]);
  void parse(const std::vector<std::string> &argStrList);

  std::string& match(const std::string &key) {
    return options_.at(key).value();
  }
  std::string match(const std::string &key) const {
    return options_.at(key).value();
  }

  void print(std::ostream &os) const;
  void info(std::ostream &os) const;
  void verbose(std::ostream &os) const;
  bool check() const;

  ArgsParser& add(const Option &option);
  ArgsParser& add(Option &&option);

  ArgsParser& operator<<(const Option &option);
  ArgsParser& operator<<(Option &&option);
private:
  std::unordered_map<std::string, Option> options_;
};

LY_NAMESPACE_END
