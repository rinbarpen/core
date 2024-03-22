#pragma once

#include <queue>
#include <optional>
#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>

LY_NAMESPACE_BEGIN
template <class T>
class AVQueue
{
public:
  AVQueue();
  explicit AVQueue(size_t max_capacity);
  ~AVQueue();

  void open();
  void close();

  bool push(const T &x);
  bool push(T &&x);
  std::optional<T> pop();
  bool pop(T &x);

  void reserve(size_t new_max_capacity);

  bool empty() const;
  bool full() const;
  size_t size() const;
  size_t capacity() const;

  bool isOpen() const { return opening_; }

private:
  bool opening_{true};
  std::queue<T> data_;
  size_t max_capacity_{0xFFFFFFFF};  // secs * framerate
  mutable Mutex::type mutex_;
};

template class AVQueue<ffmpeg::AVFramePtr>;
template class AVQueue<ffmpeg::AVPacketPtr>;

using AVFrameQueue = AVQueue<ffmpeg::AVFramePtr>;
using AVPacketQueue = AVQueue<ffmpeg::AVPacketPtr>;

LY_NAMESPACE_END
