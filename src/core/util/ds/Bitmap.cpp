#include <stdexcept>
#include <core/util/ds/Bitmap.h>

namespace ly
{
Bitmap::Bitmap(size_t nbits) 
  : nbits_(nbits) {
  data_ = new unsigned char[(nbits_ + 7) / 8];
}
Bitmap::~Bitmap() 
{
  delete[] data_;
}

void Bitmap::add(size_t n) 
{
  if (n >= nbits_) throw std::out_of_range("out of range");

  size_t bytes = n / 8;
  size_t offset = n % 8;
  (data_[bytes]) &= (1 << offset);
}

void Bitmap::or(size_t n)
{
  if (n >= nbits_) throw std::out_of_range("out of range");

  size_t bytes = n / 8;
  size_t offset = n % 8;
  (data_[bytes]) |= (1 << offset);
}

auto Bitmap::at(size_t n) const -> bool
{
  if (n >= nbits_) throw std::out_of_range("out of range");
  
  size_t bytes = n / 8;
  size_t offset = n % 8;
  return (data_[bytes] >> offset) & 1;
}
auto Bitmap::operator[](size_t n) const -> bool
{
  size_t bytes = n / 8;
  size_t offset = n % 8;
  return (data_[bytes] >> offset) & 1;
}

void Bitmap::resize(size_t size) 
{
  auto *tmp = new unsigned char[size];
  if (size >= nbits_)
    std::copy_n(tmp, data_, (nbits_ + 7) / 8);
  else
    std::copy_n(tmp, data_, size);
  
  delete[] data_;
  nbits_ = size;
  data_ = tmp;
}

auto Bitmap::toString() const -> std::string 
{
  std::string res;
  res.reserve(nbits_);
  for (size_t i = 0; i < nBytes(); ++i){ 
    for (int j = 0; j < 8 && i * 8 + j < nbits_; ++j)
    { 
      res += (data_[i] >> j) & 1;
    }
  }
  for (size_t i = 0; i < nbits_ / 2; ++i)
  { 
    std::swap(res[i], res[nbits_ - i - 1]);
  }
  return res;
}

// void Bitmap::operator<<(size_t nBits) {
//   shift_ -= nBits;
// }
// void Bitmap::operator>>(size_t nBits) {
//   shift_ += nBits;
// }


}
