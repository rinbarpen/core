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
  ~AVQueue();

  void open();
  void close();

  bool push(const T &x);
  bool push(T &&x);
  std::optional<T> pop();
  bool pop(T &x);

  bool empty() const;
  size_t size() const;

  bool isOpen() const { return opening_; }

private:
  bool opening_{true};
  std::queue<T> data_;
  mutable Mutex::type mutex_;
};

using AVFrameQueue = AVQueue<ffmpeg::AVFramePtr>;
using AVPacketQueue = AVQueue<ffmpeg::AVPacketPtr>;

LY_NAMESPACE_END
