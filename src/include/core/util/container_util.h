#pragma once

#include <algorithm>
#include <optional>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <core/util/marcos.h>

#include <fmt/core.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(container_util)

namespace detail
{
template <class Container, typename = void>
struct has_key_type : std::false_type
{};
template <class Container>
struct has_key_type<Container, std::void_t<typename Container::key_type>> :
  std::true_type
{};
template <class Container>
inline constexpr bool has_key_type_v = has_key_type<Container>::value;

template <class Container>
struct is_stl_map_like : public
  std::conditional_t<has_key_type<Container>::value,
  typename Container::key_type, typename Container::value_type>
{};
template <class Container>
using is_stl_map_like_t = typename is_stl_map_like<Container>::type;
template <class Container>
inline constexpr bool is_stl_map_like_v = has_key_type<Container>::value;
}  // namespace detail

template <class Container, typename Key = detail::is_stl_map_like_t<Container>>
static auto contain(const Container &container, const Key &key) -> bool
{
  if constexpr (detail::is_stl_map_like_v<Container>)
  {
    return container.find(key) != container.end();
  }
  else
  {
    return std::find(container.begin(), container.end(), key)
           != container.end();
  }
}

template <class Container, typename Key = detail::is_stl_map_like_t<Container>>
static auto erase_if_contain(
  Container &container, const Key &key) -> bool {
  if constexpr (detail::is_stl_map_like_v<Container>)
  {
    auto it = container.find(key);
    if (it != container.end()) {
      container.erase(it);
      return true;
    }
  }
  else
  {
    auto it = std::find(container.begin(), container.end(), key);
    if (it != container.end()) {
      container.erase(it);
      return true;
    }
  }


  return false;
}

template <class Container, typename Key = detail::is_stl_map_like_t<Container>, typename Value = std::conditional_t<detail::is_stl_map_like_v<Container>,
  typename Container::mapped_type, typename Container::value_type>>
static auto get_if_contain(const Container &container, const Key &key) -> std::optional<Value>
{
  if constexpr (detail::is_stl_map_like_v<Container>)
  {
    auto it = container.find(key);
    return (it != container.end()) ? std::make_optional(it->second) : std::nullopt;
  }
  else
  {
    auto it = std::find(container.begin(), container.end(), key);
    return (it != container.end()) ? std::make_optional(*it) : std::nullopt;
  }
}

NAMESPACE_END(container_util)

// map|unordered_map: {k1 -> v1. k2 -> v2}
// set|unordered_set: {k1, k2, ...}
// vector|array: [k1, k2, ...]
// list: k1 <-> k2 <-> k3
// forward_list: k1 -> k2 -> k3
// no stack and queue support

LY_NAMESPACE_END
