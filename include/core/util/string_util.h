#pragma once

#include <array>
#include <core/util/marcos.h>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(string_util)

#ifdef __CXX17
# include <string_view>
# define const_string_ref ::std::string_view
#else  // since cxx17
# define const_string_ref const ::std::string &
#endif

static auto split(const_string_ref s, const_string_ref delim)
  -> std::vector<std::string> {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = 0;

  while ((end = s.find(delim, start)) != std::string::npos)
  {
    result.emplace_back(s.substr(start, end - start));
    start = end + delim.length();
  }

  result.emplace_back(s.substr(start));

  return result;
}
static auto start_with(const_string_ref s, const_string_ref matchStr) -> bool {
  size_t len = matchStr.length();
  if (len > s.length())
  {
    return false;
  }

  for (size_t i = 0; i < len; ++i)
  {
    if (s[i] != matchStr[i]) return false;
  }
  return true;
}
static auto end_with(const_string_ref s, const_string_ref matchStr) -> bool {
  size_t len = matchStr.length();
  size_t slen = s.length();
  if (len > slen)
  {
    return false;
  }

  for (size_t i = 0; i < len; ++i)
  {
    if (s[slen - len + i] != matchStr[i])
    {
      return false;
    }
  }
  return true;
}

static auto ltrim(const_string_ref s) -> std::string {
  size_t len = s.length();
  if (len == 0)
  {
    return "";
  }

  size_t left = 0, right = len - 1;
  for (; left < len; ++left)
  {
    if (!isblank(s[left]))
    {
      break;
    }
  }

  if (left > right)
  {
    return "";
  }
  return std::string(s.substr(left, right - left + 1));
}
static auto rtrim(const_string_ref s) -> std::string {
  size_t len = s.length();
  if (len == 0)
  {
    return "";
  }

  size_t left = 0, right = len - 1;
  for (; right > left; --right)
    if (!isblank(s[right])) break;

  if (left > right) return "";
  return std::string(s.substr(left, right - left + 1));
}
static auto trim(const_string_ref s) -> std::string {
  size_t len = s.length();
  if (len == 0)
  {
    return "";
  }

  size_t left = 0, right = len - 1;
  for (; left < len; ++left)
  {
    if (!isblank(s[left]))
    {
      break;
    }
  }
  for (; right > left; --right)
  {
    if (!isblank(s[right]))
    {
      break;
    }
  }

  if (left > right)
  {
    return "";
  }
  return std::string(s.substr(left, right - left + 1));
}
static auto ltrim(std::string &s) -> std::string & {
  size_t len = s.length();
  if (len == 0)
  {
    return s;
  }

  size_t left = 0, right = len - 1;
  for (; left < len; ++left)
  {
    if (!isblank(s[left]))
    {
      break;
    }
  }

  if (left > right)
  {
    s = "";
    return s;
  }
  return s = s.substr(left, right - left + 1);
}
static auto rtrim(std::string &s) -> std::string & {
  size_t len = s.length();
  if (len == 0)
  {
    return s;
  }

  size_t left = 0, right = len - 1;
  for (; right > left; --right)
  {
    if (!isblank(s[right]))
    {
      break;
    }
  }

  if (left > right)
  {
    s = "";
    return s;
  }
  return s = s.substr(left, right - left + 1);
}
static auto trim(std::string &s) -> std::string & {
  size_t len = s.length();
  if (len == 0)
  {
    return s;
  }

  size_t left = 0, right = len - 1;
  for (; left < len; ++left)
  {
    if (!isblank(s[left]))
    {
      break;
    }
  }
  for (; right > left; --right)
  {

    if (!isblank(s[right]))
    {
      break;
    }
  }

  if (left > right)
  {
    s = "";
    return s;
  }
  return s = s.substr(left, right - left + 1);
}

static auto equal(const_string_ref s1, const_string_ref s2,
  bool upperLowerSensitive = false) -> bool {
  if (upperLowerSensitive)
  {
    return 0 == s1.compare(s2);
  }

  if (s1.length() != s2.length())
  {
    return false;
  }

  const char *p1 = s1.data(), *p2 = s2.data();
  for (; *p1 && *p2; ++p1, ++p2)
  {
    if (*p1 != *p2)
    {
      return false;
    }
  }
  if (*p1 || *p2)
  {
    return false;
  }

  return true;
}

static auto to_upper(const_string_ref s) -> std::string {
  std::string r;
  r.reserve(s.length());
  for (auto c : s)
  {
    r.push_back(::std::toupper(c));
  }
  return r;
}
static auto to_lower(const_string_ref s) -> std::string {
  std::string r;
  r.reserve(s.length());
  for (auto c : s)
  {
    r.push_back(::std::tolower(c));
  }
  return r;
}
static auto to_upper(std::string &s) -> std::string & {
  for (char &it : s)
  {
    it = ::std::toupper(it);
  }
  return s;
}
static auto to_lower(std::string &s) -> std::string & {
  for (char &it : s)
  {
    it = ::std::tolower(it);
  }
  return s;
}

static auto split(
  const_string_ref s, std::initializer_list<const_string_ref> delimList)
  -> std::vector<std::string> {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = 0;

  while (true)
  {
    size_t expectedEnd = std::string::npos;
    std::string_view matched;

    for (auto &delim : delimList)
    {
      expectedEnd = s.find(delim, start);
      if (expectedEnd != std::string::npos)
      {
        end = std::min(expectedEnd, end);
        matched = delim;
      }
    }
    // no matched
    if (end == std::string::npos)
    {
      result.emplace_back(s.substr(start));
      return result;
    }
    start = end + matched.length();
  }
}

static auto parse_url_params(const_string_ref url)
  -> std::unordered_map<std::string, std::string> {
  std::unordered_map<std::string, std::string> ret;

  auto &&paramString = url.substr(url.find('?') + 1);
  auto &&params = string_util::split(paramString, "&");
  for (const auto &param : params)
  {
    size_t split_index = param.find('=');
    ret.emplace(param.substr(0, split_index), param.substr(split_index + 1));
  }

  return ret;
}

static auto split_httplike_packet(const_string_ref str)
  -> std::tuple<std::string, std::string, std::string> {
  size_t curr = 0, last = curr;
  curr = str.find("\r\n", curr);
  std::string line;
  line = str.substr(last, curr - last);
  last = curr + 2;
  curr = str.rfind("\r\n");
  std::string headers;
  headers = str.substr(last, curr - last);
  std::string body;
  body = str.substr(curr + 2);

  return std::make_tuple(line, headers, body);
}
static auto split_httplike_headers(const_string_ref str)
  -> std::vector<std::string> {
  std::vector<std::string> res;

  size_t curr = 0, last = curr;
  while (true)
  {
    curr = str.find("\r\n", curr);
    if (curr == std::string::npos)
    {
      break;
    }

    res.emplace_back(str.substr(last, curr - last));
    last = curr + 2;
    curr = last;
  }

  return res;
}
static auto split_httplike_header(const_string_ref header)
  -> std::pair<std::string, std::string> {
  const size_t mid = header.find(':');
  return std::make_pair(
    trim(header.substr(0, mid)), trim(header.substr(mid + 1)));
}

NAMESPACE_BEGIN(literals)

static auto operator*(const_string_ref s, size_t n) -> ::std::string {
  ::std::string r;
  r.reserve(s.length() * n);
  for (size_t i = 0; i < n; ++i)
  {
    r.append(s);
  }
  return r;
}

NAMESPACE_END(literals)

NAMESPACE_END(string_util)

namespace literals = string_util::literals;


LY_NAMESPACE_END

#undef const_string_ref
