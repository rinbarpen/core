#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <functional>

class Record
{
public:
  template <typename Fn, typename... Args>
  static void record(std::string_view taskName, Fn &&fn, Args&&... args)
  {
    auto begin_ms = std::chrono::steady_clock::now();

    std::forward<Fn>(fn)(std::forward<Args>(args)...);

    auto end_ms = std::chrono::steady_clock::now();
    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end_ms - begin_ms);
    std::cout << "\033[34m" << taskName << "\033[0m" << " costs " << "\033[32m" << cost.count() << " ms" << "\033[0m" << std::endl;
  }

  static void compare(std::string_view taskName1, std::function<void()> task1, std::string_view taskName2, std::function<void()> task2)
  {
    auto begin_ms1 = std::chrono::steady_clock::now();
    task1();
    auto end_ms1 = std::chrono::steady_clock::now();
    auto cost1 = std::chrono::duration_cast<std::chrono::milliseconds>(end_ms1 - begin_ms1);
    std::cout << "\033[34m" << taskName1 << "\033[0m" << " costs " << "\033[32m" << cost1.count() << " ms" << "\033[0m" << std::endl;

    auto begin_ms2 = std::chrono::steady_clock::now();
    task2();
    auto end_ms2 = std::chrono::steady_clock::now();
    auto cost2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_ms2 - begin_ms2);
    std::cout << "\033[34m" << taskName1 << "\033[0m" << " costs " << "\033[32m" << cost2.count() << " ms" << "\033[0m" << std::endl;

    if (cost1 < cost2) {
      std::cout << "\033[34m" << taskName1 << "\033[0m" << " is " << "\033[31m" << "faster" << "\033[0m" <<" than " << "\033[34m" << taskName2 << "\033[0m" << std::endl;
    } else if (cost1 > cost2) {
      std::cout << "\033[34m" << taskName1 << "\033[0m" << " is " << "\033[31m" << "slower" << "\033[0m" << " than " << "\033[34m" << taskName2 << "\033[0m" << std::endl;
    } else {
      std::cout << "The cost of \033[34m" << taskName1 << "\033[0m" << " is " << "\033[31m" << "equal" << "\033[0m" << " to " << "\033[34m" << taskName2 << "\033[0m" << std::endl;
    }
  }

  void start(std::string_view taskName)
  {
    begin_ = std::chrono::steady_clock::now();
    task_name_ = taskName;
  }
  void stop()
  {
    end_ = std::chrono::steady_clock::now();
    auto cost
      = std::chrono::duration_cast<std::chrono::milliseconds>(end_ - begin_);

    std::cout << "\033[34m" << task_name_ << "\033[0m" << " costs " << "\033[32m" << cost.count() << " ms" << "\033[0m" << std::endl;
  }

  static std::chrono::steady_clock::time_point now()
  {
    return std::chrono::steady_clock::now();
  }

private:
  std::chrono::steady_clock::time_point begin_;
  std::chrono::steady_clock::time_point end_;
  std::string task_name_;
};

static void record_on(Record &record, std::string_view taskName)
{
  record.start(taskName);
}

static void record_off(Record &record)
{
  record.stop();
}

