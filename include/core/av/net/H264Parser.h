#pragma once
#include <string>

struct Nal
{
  uint8_t *start_pos, *end_pos;
};

class H264Parser
{
public:
  static Nal findNal(const uint8_t *data, uint32_t size);

private:
};

