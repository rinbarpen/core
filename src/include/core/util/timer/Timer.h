#pragma once

#include <core/util/Mutex.h>
#include <core/util/marcos.h>
#include <core/util/time/Clock.h>
#include <core/util/timer/TimerTask.h>


#include <map>
#include <thread>

LY_NAMESPACE_BEGIN

class Timer
{
public:
  Timer() = default;
  ~Timer() = default;

  static void sleep(std::chrono::milliseconds ms) {
    return ::std::this_thread::sleep_for(ms);
  }
  static TimerTask newTask(TimerTask::TaskCallback fn,
    std::chrono::milliseconds timeout, bool oneShot = true);
  static TimerTask newTask(
    TimerTask::TaskCallback fn, int timeout_ms, bool oneShot);

  bool add(const TimerTask &task);
  bool modify(const TimerTaskId id, std::chrono::milliseconds newTimeout);
  bool remove(const TimerTaskId id);

  void run();

  TimerTimestampDuration nextTimestamp() const;

  LY_NONCOPYABLE(Timer);

private:
  using KeyPair = std::pair<TimerTimestamp, TimerTaskId>;
  struct Comparator
  {
    bool operator()(const KeyPair &lhs, const KeyPair &rhs) const {
      if (lhs.first < rhs.first) {
        return true;
      }
      if (lhs.first == rhs.first) {
        return lhs.second < rhs.second;
      }
      return false;
    }
  };

  std::map<KeyPair, TimerTask, Comparator> task_map_;
  mutable Mutex::type mutex_;
};

LY_NAMESPACE_END
