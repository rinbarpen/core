#pragma once

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

template <class Container, typename = void>
struct has_key_type : std::false_type
{};
template <class Container>
struct has_key_type<Container, std::void_t<typename Container::key_type>> :
  std::true_type
{};

template <class Container>
struct stl_map_like : public
  std::conditional_t<has_key_type<Container>::value,
  typename Container::key_type, typename Container::value_type>
{};
template <class Container>
using stl_map_like_t = typename stl_map_like<Container>::type;
template <class Container>
inline constexpr bool stl_map_like_v = has_key_type<Container>::value;


NAMESPACE_BEGIN(container_util)
// TODO: Test me!
template <class Container, typename Key = stl_map_like_t<Container>>
static auto contain(const Container &container, const Key &key) -> bool
{
  if constexpr (stl_map_like_v<Container>)
  {
    return container.find(key) != container.end();
  }
  else
  {
    return std::find(container.begin(), container.end(), key)
           != container.end();
  }
}

template <class Container, typename Fn,
  typename Key = stl_map_like_t<Container>>
static auto do_something_if_contain(Container &container, 
  const Key &key, Fn &&fn) ->
  bool
{
  typename Container::iterator it;
  if constexpr (stl_map_like_v<Container>)
  {
    it = container.find(key);
  } else {
    it = std::find(container.begin(), container.end(), key);
  }

  if (it != container.end())
  {
    fn(it);
    return true;
  }   
  return false;
}

template <class Container, typename Fn,
  typename Key = stl_map_like_t<Container>>
static auto do_something_if_not_contain(
  Container &container, const Key &key, Fn &&fn) -> bool {
  typename Container::iterator it;
  if constexpr (stl_map_like_v<Container>)
  {
    it = container.find(key);
  } else {
    it = std::find(container.begin(), container.end(), key);
  }

  if (it != container.end()) { return false; }
  else { fn(); }
  return true;
}

template <class Container, typename Key = stl_map_like_t<Container>>
static auto erase_if_contain(
  Container &container, const Key &key) -> bool {
  typename Container::iterator it;
  if constexpr (stl_map_like_v<Container>)
  {
    it = container.find(key);
  }
  else
  {
    it = std::find(container.begin(), container.end(), key);
  }

  if (it != container.end()) {
    container.erase(it);
    return true;
  }
  return false;
}

template <class Container, typename Value,
  typename Key = stl_map_like_t<Container>>
static auto insert_if_not_contain(Container &container, const Key &key,
  const Value &value) -> bool {
  if constexpr (stl_map_like_v<Container>)
  {
    if (auto it = container.find(key);
        it != container.end())
    {
      return false;
    }
    container.emplace(key, value);
  }
  else
  {
    if (auto it = std::find(container.begin(), container.end(), key);
        it != container.end())
    {
      return false;
    }
    container.emplace(value);
  }

  return true;
}


NAMESPACE_END(container_util)

LY_NAMESPACE_END
