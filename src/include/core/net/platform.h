#pragma once

#if (defined(_WIN32) || defined(WIN32)) && !defined(WIN_SOCK_DEFINED)
# define WIN_SOCK_DEFINED
# define WIN32_LEAN_AND_MEAN
# define FD_SETSIZE 1024
# ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#  define _WINSOCK_DEPRECATED_NO_WARNINGS
# endif

# include <WinSock2.h>
# include <windows.h>
# include <ws2tcpip.h>
# include <iphlpapi.h>
# pragma comment(lib, "ws2_32.lib")
# pragma comment(lib, "iphlpapi.lib")

using sockfd_t = SOCKET;
#elif (defined(__linux__) || defined(__linux)) && !defined(LINUX_SOCK_DEFINED)
# define LINUX_SOCK_DEFINED

# include <sys/types.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <sys/epoll.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <netinet/ether.h>
# include <netinet/ip.h>
# include <netpacket/packet.h>
# include <arpa/inet.h>
# include <net/ethernet.h>
# include <net/route.h>
# include <net/if.h>
# include <netdb.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>

using sockfd_t = int;
# define INVALID_SOCKET ~(0)
# define SOCKET_ERROR (-1)

#else
# error "Only support win and linux"
#endif
