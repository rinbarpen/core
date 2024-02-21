#pragma once

#include <core/net/Socket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class UdpSocket : public Socket
{
public:
  UdpSocket()
    : Socket(NetProtocol::UDP)
  {}

  virtual auto type() const -> std::string override { return "UdpSocket"; }

  LY_NONCOPYABLE(UdpSocket);
};

NAMESPACE_END(net)
LY_NAMESPACE_END
