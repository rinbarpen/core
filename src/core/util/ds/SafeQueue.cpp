#include <core/util/ds/SafeQueue.h>

namespace ly
{
template <typename T>
SafeQueue<T>::SafeQueue(bool autoOpen)
{
  if (autoOpen) opening_ = true;
}
template <typename T>
SafeQueue<T>::~SafeQueue()
{
  if (opening_) opening_ = false;
}
template <typename T>
void SafeQueue<T>::open()
{
  opening_ = true;
}
template <typename T>
void SafeQueue<T>::close()
{
  opening_ = false;
}

template <typename T>
auto SafeQueue<T>::push(const T &x) -> bool
{
  if (!opening_) return false;

  Mutex::lock locker(mutex_);
  data_.push(x);
}
template <typename T>
auto SafeQueue<T>::push(T &&x) -> bool
{
  if (!opening_) return false;

  Mutex::lock locker(mutex_);
  data_.push(std::move(x));
}
template <typename T>
auto SafeQueue<T>::pop() -> std::optional<T>
{
  if (!opening_) return {};

  Mutex::lock locker(mutex_);

  T x = std::move(data_.front());
  data_.pop();
  return x;
}
template <typename T>
auto SafeQueue<T>::pop(T &x) -> bool
{
  if (!opening_) return false;

  Mutex::lock locker(mutex_);

  x = std::move(data_.front());
  data_.pop();
  return true;
}

template <typename T>
auto SafeQueue<T>::empty() const -> bool {
  Mutex::lock locker(mutex_);
  return data_.empty();
}
template <typename T>
auto SafeQueue<T>::size() const -> size_t {
  Mutex::lock locker(mutex_);
  return data_.size();
}

}  // namespace ly
