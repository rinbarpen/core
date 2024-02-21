#pragma once

#include <core/util/marcos.h>
#include <core/net/tcp/TcpConnection.h>
#include <core/multimedia/net/rtmp/RtmpServer.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class RtmpServer;

class HttpFlvConnection : public TcpConnection
{
public:
	HttpFlvConnection(std::shared_ptr<RtmpServer> rtmp_server, TaskScheduler* taskScheduler, sockfd_t sockfd);
	virtual ~HttpFlvConnection();

	bool hasFlvHeader() const
	{ return has_flv_header_; }

	bool isPlaying() const
	{ return is_playing_; }

	bool sendMediaData(uint8_t type, uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size);

	void resetKeyFrame()
	{ has_key_frame_ = false; }

private:
	friend class RtmpSession;

	bool onRead(BufferReader& buffer);
	void onClose();

	void sendFlvHeader();
	int  sendFlvTag(uint8_t type, uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size);

	std::weak_ptr<RtmpServer> rtmp_server_;
	TaskScheduler* task_scheduler_ = nullptr;
	std::string stream_path_;

	std::shared_ptr<char> avc_sequence_header_;
	std::shared_ptr<char> aac_sequence_header_;
	uint32_t avc_sequence_header_size_ = 0;
	uint32_t aac_sequence_header_size_ = 0;
	bool has_key_frame_ = false;
	bool has_flv_header_ = false;
	bool is_playing_ = false;

	static constexpr uint8_t FLV_TAG_TYPE_AUDIO = 0x8;
	static constexpr uint8_t FLV_TAG_TYPE_VIDEO = 0x9;
};


NAMESPACE_END(net)
LY_NAMESPACE_END
