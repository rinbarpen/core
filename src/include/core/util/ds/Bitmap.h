#pragma once

#include <string>

#include "core/util/marcos.h"


// TODO: Complement COW version
LY_NAMESPACE_BEGIN
// cow version
class Bitmap
{
public:
  explicit Bitmap(size_t nbits);
  ~Bitmap();

  void add(size_t n);
  void oor(size_t n);
  // Bitmap& operator&(size_t n);
  // Bitmap& operator&=(size_t n);
  // Bitmap& operator|(size_t n);
  // Bitmap& operator|=(size_t n);

  auto at(size_t n) const -> bool;
  auto operator[](size_t n) const -> bool;

  void resize(size_t size);

  auto toString() const -> std::string;
  auto nBits() const -> size_t { return nbits_; }
  auto nBytes() const -> size_t { return (nbits_ + 7) / 8; }

  // void operator<<(size_t nBits);
  // void operator>>(size_t nBits);

private:
  unsigned char *data_;
  size_t nbits_;
  // int shift_;
};

LY_NAMESPACE_END
