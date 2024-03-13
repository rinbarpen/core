#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
struct HttpRequestMessage
{
  // line
  std::string method;
  std::string url;
  std::string version;
  // headers
  std::unordered_map<std::string, std::string> headers;
  // body
  std::string body;

  std::string toString() const
  {
    std::string s;
    s += method + " " + url + " " + version + "\r\n";
    for (auto &[k, v] : headers) {
      s += k + ":" + v + "\r\n";
    }
    s += "\r\n";
    s += body;
    return s;
  }
  static HttpRequestMessage fromString(const std::string &message)
  {
    HttpRequestMessage r;
    // line
    auto curr = message.find(" ");
    auto last = 0;
    r.method = message.substr(last, curr - last);
    last = curr + 1;
    curr = message.find(" ", last);
    r.url = message.substr(last, curr - last);
    last = curr + 1;
    curr = message.find("\r\n", last);
    r.version = message.substr(last, curr - last);
    last = curr + 2;

    // headers
    while (message.substr(last, 2) != "\r\n") {
      curr = message.find(":", last);
      // LY_ASSERT(curr != std::string::npos);
      auto &&field = message.substr(last, curr - last);
      last = curr + 1;
      curr = message.find("\r\n", last);
      auto &&value = message.substr(last, curr - last);
      r.headers.emplace(field, value);
      last = curr + 2;
    }
    curr = last + 2;
    // body
    r.body = message.substr(curr);
    return r;
  }
};
struct HttpResponseMessage
{
  // line
  std::string version;
  std::string code;
  std::string status;
  // headers
  std::unordered_map<std::string, std::string> headers;
  // body
  std::string body;

  std::string toString() const
  {
    std::string s;
    s += version + " " + code + " " + status + "\r\n";
    for (auto &[k, v] : headers) {
      s += k + ":" + v + "\r\n";
    }
    s += "\r\n";
    s += body;
    return s;
  }
  static HttpResponseMessage fromString(const std::string &message)
  {
    HttpResponseMessage r;
    // line
    auto curr = message.find(" ");
    auto last = 0;
    r.version = message.substr(last, curr - last);
    last = curr + 1;
    curr = message.find(" ", last);
    r.code = message.substr(last, curr - last);
    last = curr + 1;
    curr = message.find("\r\n", last);
    r.status = message.substr(last, curr - last);
    last = curr + 2;

    // headers
    while (message.substr(last, 2) != "\r\n") {
      curr = message.find(":", last);
      // LY_ASSERT(curr != std::string::npos);
      auto &&field = message.substr(last, curr - last);
      last = curr + 1;
      curr = message.find("\r\n", last);
      auto &&value = message.substr(last, curr - last);
      r.headers.emplace(field, value);
      last = curr + 2;
    }
    curr = last + 2;
    // body
    r.body = message.substr(curr);
    return r;
  }
};

LY_NAMESPACE_END
