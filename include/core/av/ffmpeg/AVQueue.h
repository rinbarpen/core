#pragma once

#include <queue>

#include "ffmpeg_util.h"

namespace ffmpeg
{
template <class T>
class AVQueue
{
public:
  AVQueue(size_t capacity = -1) :
    capacity_(capacity)
  {}
  ~AVQueue() = default;

  void push(const T &x)
  {
    q_.push(x);
  }

  bool pop()
  {
    if (q_.empty()) return false;

    q_.pop();
    return true;
  }

  bool pop(T &x)
  {
    if (q_.empty()) return false;

    x = q_.front();
    q_.pop();
    return true;
  }

  T peek()
  {
    if (q_.empty()) return {};

    return q_.front();
  }

  bool empty() const { return q_.empty(); }
  size_t size() const { return q_.size(); }
  void setCapacity(size_t capacity) { capacity_ = capacity; }
  size_t capacity() const { return capacity_; }
private:
  size_t capacity_;
  std::queue<T> q_;
};

using AVPacketQueue = AVQueue<AVPacketPtr>;
using AVFrameQueue  = AVQueue<AVFramePtr>;

}
