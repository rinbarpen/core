#pragma once

#include <core/util/marcos.h>
#include <core/util/time/Clock.h>
#include <core/util/Mutex.h>
#include <core/util/timer/TimerTask.h>

#include <map>
#include <thread>

LY_NAMESPACE_BEGIN

class Timer
{
public:
  Timer() = default;
  ~Timer() = default;

  static void sleep(std::chrono::milliseconds ms)
  {
    return ::std::this_thread::sleep_for(ms);
  }
  static auto newTask(TimerTask::TaskCallback fn,
    std::chrono::milliseconds timeout, bool oneShot = true) -> TimerTask;

  auto add(const TimerTask& task) -> bool;
  auto modify(const TimerTaskId id, std::chrono::milliseconds newTimeout) -> bool;
  auto remove(const TimerTaskId id) -> bool;

  void run();

  LY_NODISCARD auto nextTimestamp() const -> TimerTimestampDuration;

  LY_NONCOPYABLE(Timer);
private:
  using KeyPair = std::pair<TimerTimestamp, TimerTaskId>;
  struct Comparator
  {
    bool operator()(const KeyPair &lhs, const KeyPair &rhs) const
    {
      if (lhs.first < rhs.first) {
        return true;
      }
      if (lhs.first == rhs.first)
      {
        return lhs.second < rhs.second;
      }
      return false;
    }
  };

  std::map<KeyPair, TimerTask, Comparator> task_map_;

  mutable Mutex::type mutex_;
};

LY_NAMESPACE_END
