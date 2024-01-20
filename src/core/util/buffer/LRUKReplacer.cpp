#include <chrono>
#include <core/util/buffer/LRUKReplacer.h>
#include <algorithm>

LY_NAMESPACE_BEGIN
template <class K, class V>
LRUKReplacer<K, V>::LRUKReplacer(uint8_t k, uint32_t capacity)
  : k_(k), capacity_(capacity)
{}

template <class K, class V>
auto LRUKReplacer<K, V>::put(const K& key, const V& value) -> bool
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
  if (auto it = history_.find(key); it != history_.end())
  {
    auto &tuple = it->second;
    std::get<1>(tuple) = Timestamp<T_steady_clock>::now();
    ++std::get<2>(tuple);
    if (std::get<2>(tuple) >= k_)
    {
      // move history to cache
      if (caches_.size() > capacity_)
      {
        // cache is full
        caches_.pop_back();
      }

      const V &value = std::get<0>(tuple);
      history_.erase(it);
      caches_.emplace_front(key, value);
      return false;
    }
    return false;
  }

  history_[key] = std::make_tuple(value, Timestamp<T_steady_clock>::now(), 1);
  return true;
}

template <class K, class V>
auto LRUKReplacer<K, V>::access(const K &key) -> std::optional<V>
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
  if (auto it = history_.find(key);
      it != history_.end())
  {
    auto &tuple = it->second;
    std::get<1>(tuple) = Timestamp<T_steady_clock>::now();
    ++std::get<2>(tuple);
    if (std::get<2>(tuple) >= k_) {
      // move history to cache
      if (caches_.size() > capacity_) {
        // cache is full
        caches_.pop_back();
      }

      const V &value = std::get<0>(tuple);
      history_.erase(it);
      caches_.emplace_front(key, value);
      return {};
    }
    return {};
  }

  return {};
}

template <class K, class V>
auto LRUKReplacer<K, V>::evite(const K &key) -> bool
{
  Mutex::lock locker(mutex_);
  if (auto it = std::find_if(caches_.begin(), caches_.end(),
        [&](const std::pair<K, V> &pair) { return key == pair.first; });
      it != caches_.end())
  {
    caches_.erase(it);
    return true;
  }
  if (auto it = history_.find(key);
      it != history_.end())
  {
    history_.erase(it);
    return true;
  }


  return false;
}

LY_NAMESPACE_END
