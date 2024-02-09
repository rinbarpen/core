#include <core/util/ds/Trie.h>
#include <memory>
#include <mutex>
#include <vector>

LY_NAMESPACE_BEGIN

TrieNode::TrieNode(char token) : token_(token) {}
TrieNode::TrieNode(char token, bool is_end) : token_(token), is_end_(is_end) {}
auto TrieNode::put(char token) -> std::shared_ptr<TrieNode> {
  std::lock_guard locker(mutex_);
  return children_[token] = std::make_shared<TrieNode>(token);
}
template <typename T>
auto TrieNode::putWithValue(char token, const T &value)
  -> std::shared_ptr<TrieNode> {
  std::lock_guard locker(mutex_);
  return children_[token] =
           std::make_shared<TrieNodeWithValue<T>>(token, value);
}
auto TrieNode::remove(char token) -> bool {
  std::lock_guard locker(mutex_);
  if (auto it = children_.find(token); it != children_.end())
  {
    if (it->second->hasSingleChild())
      children_.erase(it);
    else
    {
      return false;
    }
    return true;
  }
  return false;
}
template <typename T>
auto TrieNode::removeWithValue(char token) -> std::optional<T> {
  std::lock_guard locker(mutex_);
  if (auto it = children_.find(token); it != children_.end())
  {
    auto child = it->second;
    auto x = std::dynamic_pointer_cast<TrieNodeWithValue<T>>(child);
    std::optional<T> value = x->value();
    if (child->isNoChildren()) {
      children_.erase(it);
    }
    else
    {
      auto u = std::make_shared<TrieNode>(child->token());
      u->children_ = child->children();
      children_[token] = u;  // delete old one, insert new one
    }
    return value;
  }
  return {};
}
auto TrieNode::clone() const -> std::shared_ptr<TrieNode>
{
  auto x = std::make_shared<TrieNode>(token_, is_end_);
  x->children_ = this->children();
  return x;
}
auto TrieNode::child(char token) const -> std::shared_ptr<TrieNode> {
  std::lock_guard locker(mutex_);
  if (auto it = children_.find(token); it != children_.end())
  {
    return it->second;
  }
  return {};
}
void TrieNode::destroy() {
  std::lock_guard locker(mutex_);
  for (auto [_, child] : children_)
  {
    child->destroy();
  }
  children_.clear();
}

// Trie
Trie::Trie() {
  root_ = std::make_shared<TrieNode>('/');
}
Trie::~Trie() {
  root_->destroy();
}
template <typename T>
auto Trie::put(std::string_view path, const T &value) -> bool {
  std::shared_ptr<TrieNode> curr = root_;
  for (size_t i = 0; i < path.size(); ++i)
  {
    char token = path[i];
    auto pChild = curr->child(token);
    if (!pChild)
    {
      // put
      if (i == path.size() - 1)
      {
        pChild = curr->putWithValue<T>(token, value);
        return true;
      }
      else
      {
        pChild = curr->put(token);
      }
    }
    else
    {
      if (i == path.size() - 1)
      {
        auto x = std::make_shared<TrieNodeWithValue<T>>(token, value);
        x->children_ = pChild->children();
        curr->children_[token] = x;
        pChild = x;
      }
    }
    curr = pChild;
  }
  return false;
}
template <typename T>
auto Trie::get(std::string_view path) -> std::optional<T> {
  std::shared_ptr<TrieNode> curr = root_;
  for (char token : path)
  {
    auto pChild = curr->child(token);
    if (!pChild)
    {
      return {};
    }
    curr = pChild;
  }
  if (curr->hasValue())
  {
    auto x =
      std::dynamic_pointer_cast<TrieNodeWithValue<T>>(curr);
    LY_ASSERT(x);
    return x->value();
  }
  return {};
}
template <typename T>
auto Trie::removeWithValue(std::string_view path) -> std::optional<T> {
  std::shared_ptr<TrieNode> curr = root_;
  std::vector<std::pair<std::shared_ptr<TrieNode>, char>> found;
  found.reserve(path.size());

  for (char token : path)
  {
    auto pChild = curr->child(token);
    if (!pChild)
    {
      return {};
    }
    found.push_back({curr, token});
    curr = pChild;
  }

  std::optional<T> value;
  if (auto it = found.rbegin(); it != found.rend())
  {
    auto [node, token] = *it;
    value = node->removeWithValue<T>(token);
    found.pop_back();
  }

  for (auto it = found.rbegin(); it != found.rend(); ++it)
  {
    auto [node, token] = *it;
    node->remove(token);
  }

  return true;
}
auto Trie::remove(std::string_view path) -> bool {
  std::shared_ptr<TrieNode> curr = root_;
  std::vector<std::pair<std::shared_ptr<TrieNode>, char>> found;
  found.reserve(path.size());

  for (char token : path)
  {
    auto pChild = curr->child(token);
    if (!pChild)
    {
      return false;
    }
    found.push_back({curr, token});
    curr = pChild;
  }

  for (auto it = found.rbegin(); it != found.rend(); ++it)
  {
    auto [node, token] = *it;
    node->remove(token);
  }

  return true;
}

LY_NAMESPACE_END
