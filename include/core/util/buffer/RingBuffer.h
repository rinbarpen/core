#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <core/util/Mutex.h>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN

// Thread-Safe
template <typename T>
class RingBuffer
{
public:
  explicit RingBuffer(size_t capacity = 4096) :
		capacity_(capacity)
  {
		data_.resize(capacity);
  }
	~RingBuffer() = default;

	bool push(const T &x)
	{
		Mutex::lock locker(mutex_);

		if (capacity_ <= sizeInternal())
			return false;

		data_[put_pos_] = x;
		put_pos_ = (put_pos_ + 1) % capacity_;
	}
	bool push(T&& x)
	{
		Mutex::lock locker(mutex_);

		if (capacity_ <= size())
			return false;

		data_[put_pos_] = std::move(x);
		put_pos_ = (put_pos_ + 1) % capacity_;
		return true;
	}
	bool pop(T &x)
	{
		if (emptyInternal())
			return false;

		x = data_[get_pos_];
		get_pos_ = (get_pos_ + 1) % capacity_;

		return true;
	}
	
	bool full() const { Mutex::lock locker(mutex_); return fullInternal(); }
	bool empty() const { Mutex::lock locker(mutex_); return emptyInternal(); }
	size_t capacity() const { return capacity_; }
	size_t size() const { Mutex::lock locker(mutex_); return sizeInternal(); }
private:
	bool fullInternal() const { return sizeInternal() == capacity_; }
	bool emptyInternal() const { return put_pos_ == get_pos_; }
	size_t sizeInternal() const { return (capacity_ + put_pos_ - get_pos_) % capacity_; }
private:
	const size_t capacity_{0};
	size_t put_pos_{0};
	size_t get_pos_{0};
	std::vector<T> data_;

	mutable Mutex::type mutex_;
};

LY_NAMESPACE_END
