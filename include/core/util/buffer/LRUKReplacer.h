#pragma once

#include <list>
#include <unordered_map>
#include <core/util/time/Timestamp.h>
#include <core/util/buffer/Replacer.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
template <class K, class V>
class LRUKReplacer : public Replacer<K, V>
{
public:
  LRUKReplacer(uint8_t k, uint32_t capacity);

  /**
   * \brief return false:
   *          put kv pair into the cache head, if in cache
   *          move kv pair to the cache, if history ref exceeds k
   *        return true:
   *          put kv pair to the history, if not in history
   *
   *
   * \param key
   * \param value
   * \return return true if not in history and cache;
   *         return false otherwise
   */
  auto put(const K& key, const V& value) -> bool;
  /**
   * \brief put kv pair into the cache head, if in cache
   *        plus ref count if in history(move it to cache if ref count > k)
   *
   * \param key
   * \return return value if in cache;
   *         return nullopt otherwise
   */
  auto access(const K &key) -> std::optional<V> override;
  /**
   * \brief remove kv pair from the cache if in cache
   *        remove kv pair from the history if in history
   *        directly return if not in both
   *
   * \param key
   * \return return true if in cache;
   *         return false otherwise
   */
  auto evite(const K &key) -> bool override;

  auto k() const -> uint8_t { return k_; }
  auto capacity() const -> uint32_t { return capacity_; }

private:
  const uint8_t k_;
  const uint32_t capacity_;
  // my caches, store key value pair
  std::list<std::pair<K, V>> caches_;
  // key -> value and last access time and reference count
  std::unordered_map<const K, std::tuple<V, Timestamp<T_steady_clock>, int>> history_;
  Mutex::type mutex_;
};


LY_NAMESPACE_END
