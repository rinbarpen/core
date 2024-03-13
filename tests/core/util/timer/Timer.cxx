#include <core/util/time/Clock.h>
#include <core/util/timer/Timer.h>
#include <gtest/gtest.h>
#include <fmt/core.h>

using namespace ly;
using namespace std::literals;

TEST(TestTimer, Add)
{
  auto task = Timer::newTask([&](){
    fmt::println("{}, {}!", "Hello", "World");
  }, 100ms);

  while (!task.isExpired(Clock<T_steady_clock>::now())) {
    ;
  }
}
