#pragma once
#include <cstdint>
#include <string_view>
#include <string>

namespace ly
{

template <class T>
auto get_function_name() -> std::string
{
  const std::string& s = __PRETTY_FUNCTION__;
  std::string function_name("");
  auto pos3 = s.find('(');
  function_name += s.substr(0, pos3);
  function_name += "(";
  auto pos1 = s.find('T') + 4;
  auto pos2 = s.find_first_of(";]");
  function_name += s.substr(pos1, pos2 - pos1);
  function_name += ")";
  return function_name;
}

}
