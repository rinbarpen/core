#pragma once

#include <queue>
#include <atomic>
#include <core/util/Mutex.h>
#include <optional>

namespace ly
{

template <typename T>
class SafeQueue
{
public:
  SafeQueue();
  ~SafeQueue();

  void open();
  void close();

  auto push(const T& x) -> bool;
  auto push(T&& x) -> bool;
  auto pop() -> std::optional<T>;
  auto pop(T& x) -> bool;

  auto empty() const -> bool;
  auto size() const -> size_t;

  auto isOpen() const -> bool { return opening_; }

private:
  std::atomic_bool opening_{true};
  std::queue<T> data_;
  mutable Mutex::type mutex_;
};

}  // namespace ly
