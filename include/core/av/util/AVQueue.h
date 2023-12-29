#pragma once

#include <core/util/Mutex.h>
#include <optional>
#include <queue>
#include <core/av/ffmpeg/ffmpeg_util.h>
namespace ly
{
template <class T>
class AVQueue
{
public:
  AVQueue();
  ~AVQueue();

  void open();
  void close();

  auto push(const T &x) -> bool;
  auto push(T &&x) -> bool;
  auto pop() -> std::optional<T>;
  auto pop(T &x) -> bool;

  auto empty() const -> bool;
  auto size() const -> size_t;

  auto isOpen() const -> bool { return opening_; }

private:
  bool opening_{true};
  std::queue<T> data_;
  mutable Mutex::type mutex_;
};

using AVFrameQueue = AVQueue<ffmpeg::AVFramePtr>;
using AVPacketQueue = AVQueue<ffmpeg::AVPacketPtr>;

}  // namespace ly
