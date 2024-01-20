#pragma once

#include <core/util/timer/TimerTask.h>
#include <vector>
#include <optional>

// TODO: Use this instead of Timer's buildin-map

LY_NAMESPACE_BEGIN

class TimerHeap
{
public:
  explicit TimerHeap(size_t capacity)
    : capacity_(capacity)
  {
    tasks_.reserve(capacity);
  }

  auto peek() const -> std::optional<TimerTask>
  {
    if (tasks_.empty()) {
      return {};
    }

    return tasks_.front();
  }

  auto push(const TimerTask &task) -> bool
  {
    if (capacity_ <= tasks_.size()) {
      return false;
    }

    tasks_.push_back(task);
    moveUp(tasks_.size() - 1);
    return true;
  }
  auto push(TimerTask &&task) -> bool
  {
    if (capacity_ <= tasks_.size()) {
      return false;
    }

    tasks_.push_back(std::move(task));
    moveUp(tasks_.size() - 1);
    return true;
  }
  auto pop() -> std::optional<TimerTask>
  {
    if (tasks_.empty()) {
      return {};
    }

    std::optional<TimerTask> res = tasks_.front();
    moveDown(0);
    return res;
  }
  auto pop(TimerTask& task) -> bool
  {
    if (tasks_.empty()) {
      return false;
    }

    task = tasks_.front();
    moveDown(0);
    return true;
  }

  auto empty() const -> bool { return tasks_.empty(); }
  auto size() const -> size_t { return tasks_.size(); }
  auto capacity() const -> size_t { return capacity_; }

private:
  void moveUp(size_t pos)
  {
    size_t child = pos;
    size_t pa = (child - 1) / 2;
    while (isLower(tasks_[child], tasks_[pa])) {
      std::swap(tasks_[child], tasks_[pa]);
      child = pa;
      if (child == 0) {
        break;
      }
      pa = (child - 1) / 2;
    }
  }
  void moveDown(size_t pos)
  {
    size_t pa = pos;
    size_t child = pa * 2 + 1;
    while (child < tasks_.size()) {
      if (child + 1 < tasks_.size() && isLower(tasks_[child + 1], tasks_[child])) {
        ++child;
      }
      if (!isLower(tasks_[child], tasks_[pa])) {
        break;
      }

      std::swap(tasks_[child], tasks_[pa]);
      pa = child;
      child = pa * 2 + 1;
    }
  }
  bool isLower(const TimerTask &a, const TimerTask &b) const
  {
    if (a.expire_time == b.expire_time) {
      return a.id < b.id;
    }
    return a.expire_time < b.expire_time;
  }

private:
  // TimerTask.id TimerTask.expire_time
  std::vector<TimerTask> tasks_;
  const size_t capacity_;
};

LY_NAMESPACE_END
