#include <core/util/ContainerUtil.h>
#include <core/util/timer/Timer.h>
#include <core/config/config.h>


LY_NAMESPACE_BEGIN
static auto g_timeout_if_not_task_ms =
  LY_CONFIG_GET(common.timer.timeout_if_not_task_ms);

TimerTask Timer::newTask(
  TimerTask::TaskCallback fn, std::chrono::milliseconds timeout, bool oneShot) {
  return TimerTask(fn, timeout, oneShot);
}
TimerTask Timer::newTask(
  TimerTask::TaskCallback fn, int timeout_ms, bool oneShot) {
  return TimerTask(fn, timeout_ms, oneShot);
}

bool Timer::add(const TimerTask &task) {
  if (auto it = std::find_if(task_map_.begin(), task_map_.end(),
        [&](const auto &pair) { return pair.first.second == task.id; });
      it == task_map_.end()) {
    KeyPair key = {task.expire_time, task.id};
    task_map_[key] = task;
    return true;
  }
  return false;
}
bool Timer::modify(const TimerTaskId id, std::chrono::milliseconds newTimeout) {
  if (auto it = std::find_if(task_map_.begin(), task_map_.end(),
        [&](const auto &pair) { return pair.first.second == id; });
      it != task_map_.end()) {
    TimerTask task = it->second;
    task_map_.erase(it);
    task.setInterval(newTimeout);
    KeyPair key = {task.expire_time, task.id};
    task_map_[key] = task;
    return true;
  }

  return false;
}
bool Timer::remove(const TimerTaskId id) {
  if (auto it = std::find_if(task_map_.begin(), task_map_.end(),
        [&](const auto &pair) { return pair.first.second == id; });
      it != task_map_.end()) {
    task_map_.erase(it);
    return true;
  }

  return false;
}

void Timer::run() {
  const auto now = TimerClock::now();

  Mutex::ulock locker(mutex_);
  if (task_map_.empty()) {
    return;
  }
  std::vector<TimerTask> taskList;
  while (!task_map_.empty()) {
    TimerTask curTask = task_map_.begin()->second;
    if (!curTask.isExpired(now)) {
      break;
    }
    task_map_.erase(task_map_.begin());

    locker.unlock();
    curTask();  // call the task
    if (!curTask.one_shot) {
      curTask.setInterval(curTask.interval.duration());
      taskList.push_back(curTask);
    }
    locker.lock();
  }

  for (const auto &task : taskList) {
    KeyPair key = {task.expire_time, task.id};
    task_map_[key] = task;
  }
}

TimerTimestampDuration Timer::nextTimestamp() const {
  if (task_map_.empty()) {
    return std::chrono::milliseconds{g_timeout_if_not_task_ms};
  }

  auto now = TimerClock::now();
  Mutex::lock locker(mutex_);
  const TimerTask &curTask = task_map_.begin()->second;
  if (curTask.expire_time > now)
    return curTask.expire_time.duration<std::chrono::milliseconds>(now);

  return {0ms};
}

LY_NAMESPACE_END
