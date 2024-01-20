#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>
#include <random>

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/net/SocketUtil.h>
#include <core/util/Singleton.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class GlobalMultiAddress
{
public:
  auto getAddr() -> std::string
  {
		Mutex::lock locker(mutex_);
		std::string ip;
		struct sockaddr_in addr = { 0 };
		std::random_device rd;

		for (int n = 0; n <= 10; n++) {
			uint32_t range = 0xE8FFFFFF - 0xE8000100;
			addr.sin_addr.s_addr = ::htonl(0xE8000100 + (rd()) % range);
			ip = ::inet_ntoa(addr.sin_addr);  // ip

			if (ips_.find(ip) != ips_.end()) {
				ip.clear();
			}
			else {
				ips_.insert(ip);
				break;
			}
		}

		return ip;
  }

  void release(std::string ip)
  {
		Mutex::lock locker(mutex_);
		ips_.erase(ip);
  }
private:
  Mutex::type mutex_;
  std::unordered_set<std::string> ips_;
};

using SingleMultiAddress = Singleton<GlobalMultiAddress>;

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
  void clear()
  {
    ports_.clear();
  }

  void release()
  {
    SingleMultiAddress::instance()->release(ip_);
  }

private:
  std::string ip_;
  std::vector<uint16_t> ports_;
  // std::vector<std::pair<uint16_t, uint16_t>> ports_;
};



NAMESPACE_END(net)
LY_NAMESPACE_END
