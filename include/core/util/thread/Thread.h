#pragma once

#include <thread>

struct ThreadContext
{
  
};

class Thread
{
public:


private:
  ThreadContext &context_;
  std::thread thread_;
};