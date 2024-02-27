#include <cstring>

#include <core/net/SocketUtil.h>
#include <core/util/buffer/BufferWriter.h>

LY_NAMESPACE_BEGIN
BufferWriter::BufferWriter(uint32_t capacity)
	: max_queue_length_(capacity)
{

}

bool BufferWriter::append(std::shared_ptr<char> data, uint32_t size, uint32_t index)
{
	if (size <= index) {
		return false;
	}

	if (buffer_.size() >= max_queue_length_) {
		return false;
	}

	Packet pkt = { data, size, index };
	buffer_.emplace(std::move(pkt));
	return true;
}

bool BufferWriter::append(const char* data, uint32_t size, uint32_t index)
{
	if (size <= index) {
		return false;
	}

	if (buffer_.size() >= max_queue_length_) {
		return false;
	}

	Packet pkt;
	pkt.data.reset(new char[size+512], std::default_delete<char[]>());
	memcpy(pkt.data.get(), data, size);
	pkt.size = size;
	pkt.write_index = index;
	buffer_.emplace(std::move(pkt));
	return true;
}

int BufferWriter::send(sockfd_t sockfd, std::chrono::milliseconds timeout)
{
	return this->send(sockfd, timeout.count());
}
int BufferWriter::send(sockfd_t sockfd, int ms_timeout)
{
	if (ms_timeout > 0) {
		net::socket_api::set_blocking(sockfd, ms_timeout);
	}

	int r = 0;
	int count = 1;
	do {
		if (buffer_.empty()) {
			return 0;
		}

		count -= 1;
		Packet &pkt = buffer_.front();
		r = net::socket_api::send(sockfd, pkt.data.get() + pkt.write_index, pkt.size - pkt.write_index, 0);
		if (r > 0) {
			pkt.write_index += r;
			if (pkt.size == pkt.write_index) {
				count += 1;
				buffer_.pop();
			}
		}
		else if (r < 0) {
#if defined(__LINUX__)
			if (errno == EINTR || errno == EAGAIN)
#elif defined(__WIN__)
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS || error == 0)
#endif
			{
				r = 0;
			}
		}
	} while (count > 0);

	if (ms_timeout > 0) {
		net::socket_api::set_nonblocking(sockfd);
	}

	return r;
}

LY_NAMESPACE_END
