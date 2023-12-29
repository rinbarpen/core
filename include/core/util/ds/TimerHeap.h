#pragma once

#include <core/util/timer/TimerTask.h>
#include <vector>

LY_NAMESPACE_BEGIN

class TimerHeap
{
public:
  
protected:

private:
  // TimerTask.id TimerTask.expire_time
  std::vector<TimerTask> tasks_;
};

LY_NAMESPACE_END
