#pragma once

#include "marcos.h"

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(container_util)

template <class Container>
static auto contain(const Container &container,
  const typename Container::key_type &key) -> typename Container::const_iterator
{
  return std::find(container.begin(), container.end(), key);
}

template <class Container, typename Fn>
static auto doIfContain(const Container &container, const typename Container::key_type &key, Fn &&fn) ->
  bool
{
  if (auto it = std::find(container.begin(), container.end(), key);
      it != container.end())
  {
    fn(it);
    return true;
  }
  return false;
}

NAMESPACE_BEGIN(detail)

NAMESPACE_END(detail)

NAMESPACE_END(container_util)

LY_NAMESPACE_END
