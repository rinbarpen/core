#pragma once

#include <random>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
class Generic
{
public:
  Generic() = default;
  virtual ~Generic() = default;

};

template <typename T, T N = 0>
class ForwardSequenceGeneric
{
public:
  ForwardSequenceGeneric(T begin = N)
    : next_generic_num_(begin)
  {}
  ~ForwardSequenceGeneric() = default;

  auto next() -> T { return ++next_generic_num_; }

private:
  T next_generic_num_;
};

template <typename T, T N = 0>
class ReverseSequenceGeneric
{
public:
  ReverseSequenceGeneric(T begin = N)
    : prev_generic_num_(begin)
  {}
  ~ReverseSequenceGeneric() = default;

  auto prev() -> T { return --prev_generic_num_; }

private:
  T prev_generic_num_;
};

template <typename T, T N = 0>
class BiSequenceGeneric
{
public:
  BiSequenceGeneric(T begin = N)
    : current_generic_num_(begin)
  {}
  ~BiSequenceGeneric() = default;

  auto current() const -> T { return current_generic_num_; }
  auto prev() -> T { return --current_generic_num_; }
  auto next() -> T { return ++current_generic_num_; }

private:
  T current_generic_num_;
};

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
class RandomGeneric
{
public:
  RandomGeneric(T begin, T end)
    : begin_(begin), end_(end)
  {}
  ~RandomGeneric() = default;

  auto get() -> T {
    if (capacity() <= seen_.size()) {
      return end_;
    }

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<T> dis(begin_, end_);
    T value = dis(gen);

    // Check for uniqueness
    while (seen_.find(value) != seen_.end()) {
      value = dis(gen);
    }

    seen_.insert(value);
    return value;
  }

  auto capacity() const -> size_t { return end_ - begin_; }
private:
  T begin_, end_;
  std::unordered_set<T> seen_;
};

class RandomStringGeneric
{
public:
  static constexpr std::string_view kDigestAndCharactersCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static constexpr std::string_view kOnlyDigestCharset = "0123456789";
  static constexpr std::string_view kOnlyCharactersCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static constexpr std::string_view kOnlyLowerCharactersCharset = "abcdefghijklmnopqrstuvwxyz";
  static constexpr std::string_view kOnlyUpperCharactersCharset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  static auto get(std::string_view charset) -> std::string
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.length() - 1);

    std::string randomString;
    randomString.reserve(charset.length());

    for (size_t i = 0; i < charset.length(); ++i) {
      randomString += charset[dis(gen)];
    }

    return randomString;
  }
};

LY_NAMESPACE_END
