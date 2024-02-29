#pragma once
#include <string>

#include <core/util/marcos.h>
#include <fmt/core.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

struct NetAddress
{
  std::string ip;
  uint16_t port;

  auto isNull() const -> bool { return ip.empty() || port == 0; }
};

// namespace v2
// {
// struct NetAddress
// {
//   std::string ip;
//   uint16_t port;
//   bool ipv6;
// };
// }

NAMESPACE_END(net)
LY_NAMESPACE_END

template <>
struct fmt::formatter<::ly::net::NetAddress>
{
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ::ly::net::NetAddress &obj, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}:{}", obj.ip, obj.port);
  }
};
