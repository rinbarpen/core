#pragma once
#include <cstdint>

#include "Mutex.h"

class AudioBuffer
{
public:
  AudioBuffer(size_t capacity);
  ~AudioBuffer();

  int read(uint8_t* data, size_t len);
  int write(const uint8_t* data, size_t len);

  void clear();

  uint8_t* peek();
  const uint8_t* peek() const;

  size_t readableBytes() const;
  size_t writableBytes() const;
  size_t size() const;

  size_t capacity() const { return capacity_; }

private:
  size_t readableBytesNoLock() const { return write_pos_ - read_pos_; }
  size_t writableBytesNoLock() const { return capacity_ - write_pos_; }
  size_t sizeNoLock() const { return writableBytes(); }

  uint8_t* peekNoLock() const { return buffer_ + read_pos_; }

  void retrieve(size_t n);
  void retrieveAll();

private:
  uint8_t* buffer_{nullptr};
  const size_t capacity_;
  size_t read_pos_{0};
  size_t write_pos_{0};

  mutable Mutex::type mutex_;
};
