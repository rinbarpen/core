#include <core/util/buffer/LLRUReplacer.h>

LY_NAMESPACE_BEGIN
template <class K, class V>
LLRUReplacer<K, V>::LLRUReplacer(uint32_t capacity, uint32_t lru_capacity)
  : lru_capacity_(lru_capacity), capacity_(capacity)
{
  LY_ASSERT(capacity >= lru_capacity);
}

template <class K, class V>
auto LLRUReplacer<K, V>::put(const K &key, const V &value) -> bool
{
  return false;
}

template <class K, class V>
auto LLRUReplacer<K, V>::access(const K& key) -> std::optional<V>
{
  return false;
}

template <class K, class V>
auto LLRUReplacer<K, V>::evite(const K& key) -> bool
{
  return false;
}

LY_NAMESPACE_END
