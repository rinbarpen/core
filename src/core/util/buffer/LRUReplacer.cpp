#pragma once

#include <core/util/buffer/LRUReplacer.h>

LY_NAMESPACE_BEGIN
template <class K, class V>
LRUReplacer<K, V>::LRUReplacer(uint32_t capacity)
  : lru_capacity_(capacity) {
  caches_.reverse(lru_capacity_);
}

template <class K, class V>
auto LRUReplacer<K, V>::put(const K& key, const V& value) -> bool
{
  Mutex::lock locker(mutex_);
  if (auto it = std::find_if(caches_.begin(), caches_.end(),
        [&](const std::pair<K, V> &pair) { return key == pair.first; });
      it != caches_.end())
  {
    caches_.erase(it);
    caches_.emplace_front(key, value);
    return false;
  }

  if (caches_.size() >= lru_capacity_) {
    caches_.pop_back();
  }
  caches_.emplace_front(key, value);
  return true;
}

template <class K, class V>
auto LRUReplacer<K, V>::access(const K &key) -> std::optional<V>
{
  Mutex::lock locker(mutex_);
  if (auto it = std::find_if(caches_.begin(), caches_.end(),
        [&](const std::pair<K, V> &pair) { return key == pair.first; });
      it != caches_.end())
  {
    const V &value = it->second;
    caches_.erase(it);
    caches_.emplace_front(key, value);
    return value;
  }

  return {};
}

template <class K, class V>
auto LRUReplacer<K, V>::evite(const K &key) -> bool
{
  Mutex::lock locker(mutex_);
  if (auto it = std::find_if(caches_.begin(), caches_.end(),
        [&](const std::pair<K, V> &pair) { return key == pair.first; });
      it != caches_.end()) {
    caches_.erase(it);
    return true;
  }

  return false;
}


LY_NAMESPACE_END
