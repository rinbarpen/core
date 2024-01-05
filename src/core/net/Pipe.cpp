#include <core/net/Pipe.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

static auto g_net_logger = GET_LOGGER("net");

bool Pipe::create()
{
  ILOG_DEBUG(g_net_logger) << "Create a pipe";
#if defined(__WIN__)
	fds_[0] = socket_api::socket_tcp();
	fds_[1] = socket_api::socket_tcp();
	
	socket_api::setNonBlocking(fds_[0]);
	socket_api::setNonBlocking(fds_[1]);
#elif defined(__LINUX__)
	if (::pipe2(fds_, O_NONBLOCK | O_CLOEXEC) < 0) {
		return false;
	}
#endif
	return true;
}

bool Pipe::create(sockfd_t in, sockfd_t out)
{
#if defined(__WIN__)
	fds_[0] = in;
	fds_[1] = out;

	socket_api::setNonBlocking(fds_[0]);
	socket_api::setNonBlocking(fds_[1]);
#elif defined(__LINUX__)
	if (::pipe2(fds_, O_NONBLOCK | O_CLOEXEC) < 0) {
		return false;
	}
#endif
	return true;
}

int Pipe::write(void *buf, int len)
{
	int size;
	size = socket_api::send(fds_[0], (char *)buf, len, 0);
	return size;
}

int Pipe::read(void *buf, int len)
{
	int size;
	size = socket_api::recv(fds_[1], (char *)buf, len, 0);
	return size;
}

bool Pipe::close()
{
	bool r = (socket_api::close(fds_[0]) == 0) & (socket_api::close(fds_[1]) == 0);
	if (r)
		return true;

	return false;
}

NAMESPACE_END(net)
LY_NAMESPACE_END
