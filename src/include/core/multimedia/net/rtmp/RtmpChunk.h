#pragma once

#include <map>
#include <core/multimedia/net/rtmp/RtmpMessage.h>
#include <core/multimedia/net/rtmp/amf.h>
#include <core/util/buffer/BufferReader.h>
#include <core/util/buffer/BufferWriter.h>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class RtmpChunk
{
public:
  enum Status
  {
    PARSE_HEADER,
    PARSE_BODY
  };

  RtmpChunk();
  virtual ~RtmpChunk();

  int parse(BufferReader &in_buffer, RtmpMessage &out_rtmp_msg);
  int createChunk(
    uint32_t csid, RtmpMessage &rtmp_msg, char *buf, uint32_t buf_size);

  void setInChunkSize(uint32_t in_chunk_size) {
    in_chunk_size_ = in_chunk_size;
  }
  void setOutChunkSize(uint32_t out_chunk_size) {
    out_chunk_size_ = out_chunk_size;
  }
  void clear() { rtmp_messages_.clear(); }
  int getStreamId() const { return stream_id_; }

private:
	int parseChunkHeader(BufferReader& buffer);
	int parseChunkBody(BufferReader& buffer);
	int createBasicHeader(uint8_t fmt, uint32_t csid, char* buf);
	int createMessageHeader(uint8_t fmt, RtmpMessage& rtmp_msg, char* buf);

private:
	Status status_;
	int chunk_stream_id_ = 0;
	int stream_id_ = 0;
	uint32_t in_chunk_size_ = 128;
	uint32_t out_chunk_size_ = 128;
	std::map<int, RtmpMessage> rtmp_messages_;

	static constexpr int kDefaultStreamId = 1;
	static constexpr int kChunkMessageHeaderLen[4] = { 11, 7, 3, 0 };
};
NAMESPACE_END(net)
LY_NAMESPACE_END
