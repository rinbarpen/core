#pragma once

#include <core/net/SocketUtil.h>

#include <fmt/core.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class SockRuntimeException : public std::runtime_error
{
public:
  SockRuntimeException(const char *msg) : std::runtime_error(msg) {}
};

class SockBindException : public SockRuntimeException
{
public:
  SockBindException(NetAddress address) 
    : SockRuntimeException(
    ::fmt::vformat("Fail to BIND {}", address)
    .c_str()) 
  {}
};
class SockListenException : public SockRuntimeException
{
public:
  SockListenException(NetAddress address) 
    : SockRuntimeException(
    ::fmt::vformat("Fail to LISTEN in {}", address)
    .c_str()) 
  {}
};
class SockConnectException : public SockRuntimeException
{
public:
  SockConnectException(NetAddress address) 
    : SockRuntimeException(
    ::fmt::vformat("Fail to CONNECT in {}", address).c_str()) 
  {}
};
class SockAcceptException : public SockRuntimeException
{
public:
  SockAcceptException(NetAddress address)
    : SockRuntimeException(
    ::fmt::vformat("Fail to ACCEPT from {}", address).c_str()) 
  {}
};
class SockCreateException : public SockRuntimeException
{
public:
  SockCreateException() 
    : SockRuntimeException("Fail to CREATE a new socket") 
  {}
};
class SockSetupException : public SockRuntimeException
{
public:
  SockSetupException(const char *setupstr)
    : SockRuntimeException(fmt::vformat("Fail to Setup {}", setupstr)) 
  {}
};

NAMESPACE_END(net)
LY_NAMESPACE_END
