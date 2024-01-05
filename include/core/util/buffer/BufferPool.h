#pragma once

#include <core/util/buffer/Replacer.h>
#include <core/util/buffer/LRUKReplacer.h>

LY_NAMESPACE_BEGIN

template <class K, class V>
class BufferPool
{
public:
  BufferPool(std::unique_ptr<Replacer<K, V>> replacer) 
    : replacer_(std::move(replacer)) 
  {}

  auto access(const K& key) -> std::optional<V>;
  auto evite(const K& key) -> bool;

private:
  std::unique_ptr<Replacer<K, V>> replacer_;
};

LY_NAMESPACE_END
