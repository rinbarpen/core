#pragma once
#include <core/net/Socket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class TcpSocket : public Socket
{
public:
  TcpSocket() : Socket(NetProtocol::TCP) {}

  int setNoDelay(int on = 1);

  auto type() const -> std::string override { return "TcpSocket"; }

  LY_NONCOPYABLE(TcpSocket);
};

NAMESPACE_END(net)
LY_NAMESPACE_END
