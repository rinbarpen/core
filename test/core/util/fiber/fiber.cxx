#include <core/util/fiber/Fiber.h>
#include <core/util/fiber/FiberScheduler.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ly;

class Logic : public Fiber
{
public:
  Logic(FiberScheduler* scheduler) : Fiber(scheduler) {}
  ~Logic() override { 
    Fiber::~Fiber();
  }

  virtual void run() override 
  { 
    for (size_t i = 0; i < 10; ++i)
    {
      result = i;
      yield();    
    }
  }
  
  int result;
};

class Logic2 : public Fiber
{
public:
  Logic2(FiberScheduler *scheduler) : Fiber(scheduler) {}
  ~Logic2() override { Fiber::~Fiber(); }

  virtual void run() override
  {
    for (size_t i = 9; i >= 0; --i)
    {
      result = i;
      yield();
    }
  }

  int result;
};

int main() 
{
  FiberScheduler scheduler;
  auto logic1 = std::make_shared<Logic>(&scheduler);
  auto logic2 = std::make_shared<Logic2>(&scheduler);
  scheduler.track(logic1);
  scheduler.track(logic2);

  while (true)
  {
    if (scheduler.currentFiber()->getStatus() == FiberStatus::DEAD) break;
    scheduler.resume(logic1->id());
    scheduler.resume(logic2->id());
  }
}
