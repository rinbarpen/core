#pragma once

#include <cstdint>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
// chunk header: basic_header + rtmp_message_header
struct RtmpMessageHeader
{
  uint8_t timestamp[3];
  uint8_t length[3];
  uint8_t type_id;
  uint8_t stream_id[4];  // little endian
};

struct RtmpMessage
{
	uint32_t timestamp = 0;
	uint32_t length = 0;
	uint8_t  type_id = 0;
	uint32_t stream_id = 0;
	uint32_t extend_timestamp = 0;

	uint64_t long_timestamp = 0;
	uint8_t  codec_id = 0;

	uint8_t  csid = 0;
	uint32_t index = 0;
	std::shared_ptr<char> payload = nullptr;

  void clear() {
    index = 0;
		timestamp = 0;
		extend_timestamp = 0;
		if (length > 0) {
			payload.reset(new char[length], std::default_delete<char[]>());
		}
  }
  bool isCompleted() const {
		return index == length
        && length > 0
        && payload != nullptr;
  }
};

LY_NAMESPACE_END
