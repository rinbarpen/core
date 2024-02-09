#pragma once

#include "core/util/marcos.h"
#include <memory>
#include <unordered_map>
#include <optional>
#include <string_view>
#include <atomic>
#include <mutex>


LY_NAMESPACE_BEGIN

class TrieNode
{
  friend class Trie;
public:
  explicit TrieNode(char token);
  TrieNode(char token, bool is_end);

  void destroy();

  auto put(char token) -> std::shared_ptr<TrieNode>;
  template <typename T>
  auto putWithValue(char token, const T& value) -> std::shared_ptr<TrieNode>;
  auto remove(char token) -> bool;
  template <typename T>
  auto removeWithValue(char token) -> std::optional<T>;

  auto child(char token) const -> std::shared_ptr<TrieNode>;

  auto token() const -> char { return token_; }
  auto clone() const -> std::shared_ptr<TrieNode>;
  auto hasValue() const -> bool { return is_end_; }
  auto children() const -> std::unordered_map<char, std::shared_ptr<TrieNode>> { return children_; }
  auto count() const -> size_t { return children_.size(); }
  auto isNoChildren() const -> bool { return children_.empty(); }
  auto hasSingleChild() const -> bool { return children_.size() == 1; }

  LY_NONCOPYABLE(TrieNode);

private:
  mutable std::mutex mutex_;
  const char token_;
  std::atomic_bool is_end_{false};
  std::unordered_map<char, std::shared_ptr<TrieNode>> children_;
};
template <typename T>
class TrieNodeWithValue : public TrieNode
{
  friend class Trie;
public:
  TrieNodeWithValue(char token, const T& value)
    : TrieNode(token, true), value_(value)
  {}
  TrieNodeWithValue(char token, T&& value)
    : TrieNode(token, true), value_(std::move(value))
  {}

  auto value() -> T& { return value_; }
private:
  T value_;
};

class Trie
{
public:
  Trie();
  ~Trie();

  template <typename T>
  auto put(std::string_view path, const T& value) -> bool;
  template <typename T>
  auto get(std::string_view path) -> std::optional<T>;
  template <typename T>
  auto removeWithValue(std::string_view path) -> std::optional<T>;
  auto remove(std::string_view path) -> bool;

private:
  std::shared_ptr<TrieNode> root_;
};

LY_NAMESPACE_END
