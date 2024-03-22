#include <core/multimedia/util/AVQueue.h>

LY_NAMESPACE_BEGIN

template <typename T>
AVQueue<T>::AVQueue() {
}
template <typename T>
AVQueue<T>::AVQueue(size_t max_capacity) :
  max_capacity_(max_capacity)
{}
template <typename T>
AVQueue<T>::~AVQueue() {
  this->close();
}
template <typename T>
void AVQueue<T>::open() {
  opening_ = true;
}
template <typename T>
void AVQueue<T>::close() {
  opening_ = false;
}

template <typename T>
bool AVQueue<T>::push(const T &x) {
  if (!opening_) return false;

  Mutex::lock locker(mutex_);
  if (data_.size() >= max_capacity_) {
    data_.pop();
  }
  data_.push(x);
  return true;
}
template <typename T>
bool AVQueue<T>::push(T &&x) {
  if (!opening_) return false;

  Mutex::lock locker(mutex_);
  if (data_.size() >= max_capacity_) {
    data_.pop();
  }
  data_.push(std::move(x));
  return true;
}
template <typename T>
std::optional<T> AVQueue<T>::pop() {
  if (!opening_) return {};

  Mutex::lock locker(mutex_);

  T x = std::move(data_.front());
  data_.pop();
  return x;
}
template <typename T>
bool AVQueue<T>::pop(T &x) {
  if (!opening_) return false;

  Mutex::lock locker(mutex_);

  x = std::move(data_.front());
  data_.pop();
  return true;
}

template <typename T>
void AVQueue<T>::reserve(size_t new_max_capacity) {
  Mutex::lock locker(mutex_);
  max_capacity_ = new_max_capacity;
}

template <typename T>
bool AVQueue<T>::empty() const {
  Mutex::lock locker(mutex_);
  return data_.empty();
}
template <typename T>
bool AVQueue<T>::full() const {
  Mutex::lock locker(mutex_);
  return data_.size() == max_capacity_;
}
template <typename T>
size_t AVQueue<T>::size() const {
  Mutex::lock locker(mutex_);
  return data_.size();
}
template <typename T>
size_t AVQueue<T>::capacity() const {
  Mutex::lock locker(mutex_);
  return max_capacity_;
}

LY_NAMESPACE_END
