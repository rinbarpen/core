#pragma once

#include "core/net/SocketUtil.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class MulticastAddress
{
public:
  MulticastAddress(std::string_view ip, std::initializer_list<uint16_t> ports) noexcept
    : ip_(ip), ports_(std::move(ports))
  {}
  MulticastAddress(std::string_view ip, uint32_t nports) noexcept
    : ip_(ip)
  {
    ports_.reserve(nports);
  }

  auto ip() const noexcept -> std::string { return ip_; }
  auto hasPort(uint16_t target) const noexcept -> bool {
    return std::find(ports_.cbegin(), ports_.cend(), target) != ports_.cend();
  }
  auto port(uint32_t index) const noexcept -> uint16_t
  {
    if (index < ports_.size()) {
      return ports_[index];
    }
    return -1;
  }
  auto addPort(uint16_t newPort) noexcept -> bool {
    if (hasPort(newPort)) {
      return false;
    }
    ports_.push_back(newPort);
    return true;
  }

  void release()
  {
    socket_api::close();
  }

private:
  std::string ip_;
  std::vector<uint16_t> ports_;
  // std::vector<std::pair<uint16_t, uint16_t>> ports_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
