#pragma once

namespace ly
{
class nonmoveable
{
protected:
  nonmoveable() = default;
  ~nonmoveable() = default;

  nonmoveable(nonmoveable const&) = default;
  nonmoveable& operator=(nonmoveable const&) = default;

public:
  nonmoveable(nonmoveable&&) = delete;
  nonmoveable& operator=(nonmoveable&&) = delete;
};
}  // namespace ly
