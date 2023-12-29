#pragma once

#include <list>
#include <core/util/buffer/Replacer.h>
#include <core/util/Mutex.h>

LY_NAMESPACE_BEGIN
template <class K, class V>
class LRUReplacer : public Replacer<K, V>
{
public:
  explicit LRUReplacer(uint32_t capacity);

  /**
   * \brief put kv pair into the cache head, if in cache
   *        move kv pair to the cache, if not in cache
   *
   * \param key 
   * \param value 
   * \return return true if not in cache;
   *         return false otherwise
   */
  auto put(const K& key, const V& value) -> bool override;
  /**
   * \brief put kv pair into the cache head, if in cache
   *
   * \param key 
   * \return return value if in cache;
   *         return nullopt otherwise
   */
  auto access(const K& key) -> std::optional<V> override;
  /**
   * \brief remove kv pair from the cache, if in cache
   *        directly return if not in cache
   *
   * \param key
   * \return return true if in cache;
   *         return false otherwise
   */
  auto evite(const K& key) -> bool override;

  auto capacity() const -> uint32_t { return lru_capacity_; }
  
private:
  Mutex::type mutex_;
  std::list<std::pair<K, V>> caches_;
  uint32_t lru_capacity_;
};



LY_NAMESPACE_END
