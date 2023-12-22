#pragma once

#include "marcos.h"
#ifdef __WIN__
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <Winerror.h>
#define WIN32_LEAN_AND_MEAN
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma comment(lib, "Ws2_32.lib");

using sockfd_t = SOCKET;

#elif __LINUX__

#include <sys/types.h>         
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <netinet/ether.h>   
#include <netinet/ip.h>  
#include <netpacket/packet.h>   
#include <arpa/inet.h>
#include <net/ethernet.h>   
#include <net/route.h>  
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using sockfd_t = int;
#define INVALID_SOCKET ~(0)
#define SOCKET_ERROR (-1)

#endif