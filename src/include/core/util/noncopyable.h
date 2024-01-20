#pragma once

namespace ly
{
class noncopyable
{
protected:
  noncopyable() = default;
  ~noncopyable() = default;

  noncopyable(noncopyable&&) = default;
  noncopyable& operator=(noncopyable&&) = default;

public:
  noncopyable(noncopyable const&) = delete;
  noncopyable& operator=(noncopyable const&) = delete;
};
}  // namespace ly
