#pragma once
#include <core/net/SocketUtil.h>

LY_NAMESPACE_BEGIN

class Buffer
{
public:
	static constexpr int kMaxBytesPerRead = 4096;
public:
	explicit Buffer(size_t capacity = kMaxBytesPerRead);
	~Buffer();

  /**
   * \brief read buffer to s
   * \param[out] s 
   * \param[in] n 
   * \return the bytes we read actually
   */
  size_t read(char *s, size_t n);
	/**
   * \brief write buffer from s
	 * \param[in] s
	 * \param[in] n
   * \return the bytes we write actually
   */
	size_t write(const char *s, size_t n);
  /**
   * \brief read from fd
   * \param fd 
   * \return the bytes we read actually
   */
  int read(sockfd_t fd);
	/**
   * \brief write to fd
   * \param fd
   * \return the bytes we write actually
   */
	int write(sockfd_t fd, size_t size, int ms_timeout = 0);

	void reset(const size_t newCapacity);

  int find(const char *target);

  char *peek() { return data_ + get_pos_; }
  const char *peek() const { return data_ + get_pos_; }

	size_t readableBytes() const { return size(); }
	size_t writableBytes() const { return capacity_ - size(); }

  void clear() { get_pos_ = put_pos_ = 0; }
	bool full() const { return size() == capacity_; }
	bool empty() const { return put_pos_ == get_pos_; }
	size_t size() const { return (capacity_ + 1 + put_pos_ - get_pos_) % (capacity_ + 1); }
	size_t capacity() const { return capacity_; }

private:
	inline size_t indexOf(const size_t pos, const size_t n) const { return (pos + n) % (capacity_ + 1); }

private:
	size_t capacity_{ 0 };
	size_t put_pos_{ 0 };
	size_t get_pos_{ 0 };
	char *data_;
};

LY_NAMESPACE_END
