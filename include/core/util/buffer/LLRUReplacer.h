#pragma once
#include <list>
#include <core/util/buffer/Replacer.h>

LY_NAMESPACE_BEGIN
template <class K, class V>
class LLRUReplacer : public Replacer<K, V>
{
public:
  LLRUReplacer(uint32_t capacity, uint32_t lru_capacity);

  auto put(const K& key, const V& value) -> bool override;
  auto access(const K &key) -> std::optional<V> override;
  auto evite(const K &key) -> bool override;

private:
  std::list<std::pair<K, V>> caches_;
  const uint32_t lru_capacity_;
  const uint32_t capacity_;
};


LY_NAMESPACE_END
