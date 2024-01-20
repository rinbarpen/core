#pragma once

#include <core/util/marcos.h>
#include <optional>

LY_NAMESPACE_BEGIN

template <class K, class V>
class Replacer
{
public:
  Replacer() = default;
  virtual ~Replacer() = default;

  virtual auto put(const K &key, const V &value) -> bool = 0;
  virtual auto access(const K &key) -> std::optional<V> = 0;
  virtual auto evite(const K &key) -> bool = 0;

};

LY_NAMESPACE_END
