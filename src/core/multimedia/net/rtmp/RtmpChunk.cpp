#include <cstring>
#include <core/util/buffer/BufferReader.h>
#include <core/util/buffer/BufferWriter.h>
#include <core/multimedia/net/rtmp/rtmp.h>
#include <core/multimedia/net/rtmp/RtmpChunk.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
RtmpChunk::RtmpChunk()
  : status_(PARSE_HEADER),
    chunk_stream_id_(-1),
    stream_id_(kDefaultStreamId)
{}
RtmpChunk::~RtmpChunk(){}

int RtmpChunk::parse(BufferReader &in_buffer, RtmpMessage &out_rtmp_msg) {
  int r{0};
  if (in_buffer.readableBytes() == 0) { return 0; }

  switch (status_) {
  case PARSE_HEADER:
    r = parseChunkHeader(in_buffer);
    break;
  case PARSE_BODY:
    r = parseChunkBody(in_buffer);
    if (r > 0 && chunk_stream_id_ >= 0) {
      auto &rtmp_msg = rtmp_messages_[chunk_stream_id_];
      if (rtmp_msg.index == rtmp_msg.length) {
        if (rtmp_msg.timestamp >= 0xFFFFFF) {
          rtmp_msg.long_timestamp += rtmp_msg.extend_timestamp;
        }
        else {
          rtmp_msg.long_timestamp += rtmp_msg.timestamp;
        }

        out_rtmp_msg = rtmp_msg;
        chunk_stream_id_ = -1;
        rtmp_msg.clear();
      }
    }
  }

  return r;
}
int RtmpChunk::createChunk(
  uint32_t csid, RtmpMessage &rtmp_msg, char *buf, uint32_t buf_size) {
	uint32_t buf_offset = 0;
	uint32_t payload_offset = 0;
	uint32_t capacity = rtmp_msg.length + rtmp_msg.length / out_chunk_size_ * 5;
	if (buf_size < capacity) {
		return -1;
	}

	buf_offset += createBasicHeader(0, csid, buf + buf_offset);  // first chunk
	buf_offset += createMessageHeader(0, rtmp_msg, buf + buf_offset);
	if (rtmp_msg.long_timestamp >= 0xFFFFFF) {
		writeU32Forward(buf + buf_offset, (uint32_t)rtmp_msg.long_timestamp);
		buf_offset += 4;
	}

	while (rtmp_msg.length > 0) {
		if (rtmp_msg.length <= out_chunk_size_) {
			memcpy(buf + buf_offset, rtmp_msg.payload.get() + payload_offset, rtmp_msg.length);
			buf_offset += rtmp_msg.length;
			rtmp_msg.length = 0;
			break;
		}

		memcpy(buf + buf_offset, rtmp_msg.payload.get() + payload_offset, out_chunk_size_);
		payload_offset += out_chunk_size_;
		buf_offset += out_chunk_size_;
		rtmp_msg.length -= out_chunk_size_;

		buf_offset += createBasicHeader(3, csid, buf + buf_offset);
		if (rtmp_msg.long_timestamp >= 0xFFFFFF) {
			writeU32Forward(buf + buf_offset, (uint32_t)rtmp_msg.long_timestamp);
			buf_offset += 4;
		}
	}

	return buf_offset;
}

int RtmpChunk::parseChunkHeader(BufferReader& buffer)
{
	uint32_t bytesUsed = 0;
	uint8_t* buf = (uint8_t*)buffer.peek();
	uint32_t bufSize = buffer.readableBytes();

	uint8_t flags = buf[bytesUsed];
	bytesUsed += 1;

	uint8_t csid = flags & 0x3F; // chunk stream id
	if (csid == 0) { // csid [64, 319]
		if (bufSize < (bytesUsed + 2)) {
			return 0;
		}

		csid += buf[bytesUsed] + 64;
		bytesUsed += 1;
	}
	else if (csid == 1) { // csid [64, 65599]
		if (bufSize < (3 + bytesUsed)) {
			return 0;
		}
		csid += buf[bytesUsed + 1] * 255 + buf[bytesUsed] + 64;
		bytesUsed += 2;
	}

	uint8_t fmt = (flags >> 6); // message header type
	if (fmt >= 4) {
		return -1;
	}

	uint32_t headerLen = kChunkMessageHeaderLen[fmt]; // basic_header + message_header
	if (bufSize < (headerLen + bytesUsed)) {
		return 0;
	}

	RtmpMessageHeader header;
	memset(&header, 0, sizeof(RtmpMessageHeader));
	memcpy(&header, buf + bytesUsed, headerLen);
	bytesUsed += headerLen;

	auto& rtmp_msg = rtmp_messages_[csid];
	chunk_stream_id_ = rtmp_msg.csid = csid;

	if (fmt == RTMP_CHUNK_TYPE_0 || fmt == RTMP_CHUNK_TYPE_1) {
		uint32_t length = readU24Forward((const char*)header.length);
		if (rtmp_msg.length != length || !rtmp_msg.payload) {
			rtmp_msg.length = length;
			rtmp_msg.payload.reset(new char[rtmp_msg.length], std::default_delete<char[]>());
		}
		rtmp_msg.index = 0;
		rtmp_msg.type_id = header.type_id;
	}

	if (fmt == RTMP_CHUNK_TYPE_0) {
		rtmp_msg.stream_id = readU24Reverse((const char*)header.stream_id);
	}

	uint32_t timestamp = readU24Forward((const char*)header.timestamp);
	uint32_t extend_timestamp = 0;
	if (timestamp >= 0xFFFFFF || rtmp_msg.timestamp >= 0xFFFFFF) {
		if (bufSize < (4 + bytesUsed)) {
			return 0;
		}
		extend_timestamp = readU32Forward((const char*)buf + bytesUsed);
		bytesUsed += 4;
	}

	if (rtmp_msg.index == 0) { // first chunk
		if (fmt == RTMP_CHUNK_TYPE_0) {
			// absolute timestamp
			rtmp_msg.long_timestamp = 0;
			rtmp_msg.timestamp = timestamp;
			rtmp_msg.extend_timestamp = extend_timestamp;
		}
		else {
			// relative timestamp (timestamp delta)
			if (rtmp_msg.timestamp >= 0xFFFFFF) {
				rtmp_msg.extend_timestamp += extend_timestamp;
			}
			else {
				rtmp_msg.timestamp += timestamp;
			}
		}
	}

	status_ = PARSE_BODY;
	buffer.advance(bytesUsed);
	return bytesUsed;
}
int RtmpChunk::parseChunkBody(BufferReader& buffer)
{
	uint32_t bytesUsed = 0;
	uint8_t* buf = (uint8_t*)buffer.peek();
	uint32_t bufSize = buffer.readableBytes();

	if (chunk_stream_id_ < 0) {
		return -1;
	}

	auto& rtmp_msg = rtmp_messages_[chunk_stream_id_];
	uint32_t chunk_size = rtmp_msg.length - rtmp_msg.index;

	if (chunk_size > in_chunk_size_) {
		chunk_size = in_chunk_size_;
	}
	if (bufSize < (chunk_size + bytesUsed)) {
		return 0;
	}
	if (rtmp_msg.index + chunk_size > rtmp_msg.length) {
		return -1;
	}

	memcpy(rtmp_msg.payload.get() + rtmp_msg.index, buf + bytesUsed, chunk_size);
	bytesUsed += chunk_size;
	rtmp_msg.index += chunk_size;

	if (rtmp_msg.index >= rtmp_msg.length
	 || rtmp_msg.index % in_chunk_size_ == 0) {
		status_ = PARSE_HEADER;
	}

	buffer.advance(bytesUsed);
	return bytesUsed;
}
int RtmpChunk::createBasicHeader(uint8_t fmt, uint32_t csid, char* buf)
{
	int len = 0;

	if (csid >= 64 + 255) {
		buf[len++] = (fmt << 6) | 1;
		buf[len++] = (csid - 64) & 0xFF;
		buf[len++] = ((csid - 64) >> 8) & 0xFF;
	}
	else if (csid >= 64) {
		buf[len++] = (fmt << 6) | 0;
		buf[len++] = (csid - 64) & 0xFF;
	}
	else {
		buf[len++] = (fmt << 6) | csid;
	}
	return len;
}
int RtmpChunk::createMessageHeader(uint8_t fmt, RtmpMessage& rtmp_msg, char* buf)
{
	int len = 0;

	if (fmt <= 2) {
		if (rtmp_msg.long_timestamp < 0xFFFFFF) {
			writeU24Forward(buf, (uint32_t)rtmp_msg.long_timestamp);
		}
		else {
			writeU24Forward(buf, 0xFFFFFF);
		}
		len += 3;
	}
	if (fmt <= 1) {
		writeU24Forward(buf + len, rtmp_msg.length);
		len += 3;
		buf[len++] = rtmp_msg.type_id;
	}
	if (fmt == 0) {
		writeU32Reverse(buf + len, rtmp_msg.stream_id);
		len += 4;
	}

	return len;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
