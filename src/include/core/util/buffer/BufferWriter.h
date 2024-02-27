#pragma once

#include <chrono>
#include <cstdint>
#include <queue>

#include <core/util/marcos.h>
#include <core/net/platform.h>

LY_NAMESPACE_BEGIN
class BufferWriter
{
public:
	BufferWriter(uint32_t capacity = kMaxQueueLength);
	~BufferWriter() {}

	bool append(std::shared_ptr<char> data, uint32_t size, uint32_t index=0);
	bool append(const char* data, uint32_t size, uint32_t index=0);
	int send(sockfd_t sockfd, std::chrono::milliseconds timeout);
	int send(sockfd_t sockfd, int ms_timeout = 0);

	bool empty() const
	{ return buffer_.empty(); }

	bool full() const
	{ return buffer_.size() >= max_queue_length_; }

	uint32_t size() const
	{ return (uint32_t)buffer_.size(); }

private:
	typedef struct
	{
		std::shared_ptr<char> data;
		uint32_t size;
		uint32_t write_index;
	} Packet;

	std::queue<Packet> buffer_;
	uint32_t max_queue_length_ = 0;

	static constexpr uint32_t kMaxQueueLength = 10000;
};

LY_NAMESPACE_END
