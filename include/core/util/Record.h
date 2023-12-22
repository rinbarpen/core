#pragma once
#include <chrono>
#include <iostream>

class Record
{
public:
  template <typename Fn, typename... Args>
  static void record(Fn &&fn, Args&&... args)
  {
    auto begin_ms = std::chrono::high_resolution_clock::now();

    std::forward<Fn>(fn)(std::forward<Args>(args)...);

    auto end_ms = std::chrono::high_resolution_clock::now();
    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end_ms - begin_ms);
    std::cout << "This task costs " << cost.count() << " ms";
  }

  void start()
  {
    begin_ = now();
  }
  void stop()
  {
    end_ = now();
    auto cost
      = std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - begin_);
    std::cout << "This task costs " << cost.count() << " ms";
  }

  static std::chrono::steady_clock::time_point now()
  {
    return std::chrono::steady_clock::now();
  }

private:
  std::chrono::steady_clock::time_point begin_;
  std::chrono::steady_clock::time_point end_;
};

static void record_on(Record &record)
{
  record.start();
}

static void record_off(Record &record)
{
  record.stop();
}

