#include <core/util/StringUtil.h>
#include <core/util/buffer/ByteConverter.h>
#include <core/util/logger/Logger.h>
#include <core/multimedia/net/http/HttpFlvConnection.h>


LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_flv_logger = GET_LOGGER("multimedia.http.flv");

HttpFlvConnection::HttpFlvConnection(std::shared_ptr<RtmpServer> rtmp_server, TaskScheduler* scheduler, sockfd_t sockfd)
	: TcpConnection(scheduler, sockfd)
	, rtmp_server_(rtmp_server)
	, task_scheduler_(scheduler)
{
	this->setReadCallback([this](std::shared_ptr<TcpConnection> conn, BufferReader& buffer) {
		return this->onRead(buffer);
	});

	this->setCloseCallback([this](std::shared_ptr<TcpConnection> conn) {
		this->onClose();
	});
}

HttpFlvConnection::~HttpFlvConnection()
{

}

bool HttpFlvConnection::onRead(BufferReader& buffer)
{
	int offset = buffer.findLast("\r\n\r\n");
	if (offset < 0) {
		return buffer.readableBytes() < 4096;
	}

	std::string buf(buffer.peek(), offset);
	buffer.advance(offset + 4);

	auto pos1 = buf.find("GET");
	auto pos2 = buf.find(".flv");
	if (pos1 == std::string::npos || pos2 == std::string::npos || pos2 < pos1) {
		return false;
	}

	stream_path_ = string_util::trim(buf.substr(pos1 + 3, pos2 - 3 - pos1), " ");

  ILOG_DEBUG(g_flv_logger) << "stream path is " << stream_path_;

	auto rtmp_server = rtmp_server_.lock();
	if (rtmp_server) {
		std::string http_header = "HTTP/1.1 200 OK\r\nContent-Type: video/x-flv\r\n\r\n";
		this->send(http_header.c_str(), (uint32_t)http_header.size());

		auto session = rtmp_server->getSession(stream_path_);
		if (session != nullptr) {
			session->addHttpClient(std::dynamic_pointer_cast<HttpFlvConnection>(shared_from_this()));
		}
	}
	else {
		return false;
	}

	return true;
}

void HttpFlvConnection::onClose()
{
	auto rtmp_server = rtmp_server_.lock();
	if (rtmp_server != nullptr) {
		auto session = rtmp_server->getSession(stream_path_);
		if (session != nullptr) {
			auto conn = std::dynamic_pointer_cast<HttpFlvConnection>(shared_from_this());
			task_scheduler_->addTimer({[session, conn] {
				session->removeHttpClient(conn);
				return false;
			}, 1ms});
		}
	}
}


bool HttpFlvConnection::sendMediaData(uint8_t type, uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size)
{
	if (payload_size == 0) {
		return false;
	}

	is_playing_ = true;

	if (type == RTMP_AVC_SEQUENCE_HEADER) {
		avc_sequence_header_ = payload;
		avc_sequence_header_size_ = payload_size;
		return true;
	}
	else if (type == RTMP_AAC_SEQUENCE_HEADER) {
		aac_sequence_header_ = payload;
		aac_sequence_header_size_ = payload_size;
		return true;
	}

	auto conn = std::dynamic_pointer_cast<HttpFlvConnection>(shared_from_this());
	task_scheduler_->addTriggerEvent([conn, type, timestamp, payload, payload_size] {
		if (type == RTMP_VIDEO) {
			if (!conn->has_key_frame_) {
				uint8_t frame_type = (payload.get()[0] >> 4) & 0x0F;
				uint8_t codec_id = payload.get()[0] & 0x0F;
				if (frame_type == 1 && codec_id == RTMP_CODEC_ID_H264) {
					conn->has_key_frame_ = true;
				}
				else {
					return ;
				}
			}

			if (!conn->has_flv_header_) {
				conn->sendFlvHeader();
				conn->sendFlvTag(conn->FLV_TAG_TYPE_VIDEO, 0, conn->avc_sequence_header_, conn->avc_sequence_header_size_);
				conn->sendFlvTag(conn->FLV_TAG_TYPE_AUDIO, 0, conn->aac_sequence_header_, conn->aac_sequence_header_size_);
			}

			conn->sendFlvTag(conn->FLV_TAG_TYPE_VIDEO, timestamp, payload, payload_size);
		}
		else if (type == RTMP_AUDIO) {
			if (!conn->has_key_frame_ && conn->avc_sequence_header_size_ > 0) {
				return ;
			}

			if (!conn->has_flv_header_) {
				conn->sendFlvHeader();
				conn->sendFlvTag(conn->FLV_TAG_TYPE_AUDIO, 0, conn->aac_sequence_header_, conn->aac_sequence_header_size_);
			}

			conn->sendFlvTag(conn->FLV_TAG_TYPE_AUDIO, timestamp, payload, payload_size);
		}
	});

	return true;
}

void HttpFlvConnection::sendFlvHeader()
{
	char flv_header[9] = { 0x46, 0x4c, 0x56, 0x01, 0x00, 0x00, 0x00, 0x00, 0x09 };

	if (avc_sequence_header_size_ > 0) {
		flv_header[4] |= 0x1;
	}

	if (aac_sequence_header_size_ > 0)  {
		flv_header[4] |= 0x4;
	}

	this->send(flv_header, 9);
	char previous_tag_size[4] = { 0x0, 0x0, 0x0, 0x0 };
	this->send(previous_tag_size, 4);

	has_flv_header_ = true;
}

int HttpFlvConnection::sendFlvTag(uint8_t type, uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size)
{
	if (payload_size == 0) {
		return -1;
	}

	char tag_header[11] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	char previous_tag_size[4] = { 0x0, 0x0, 0x0, 0x0 };

	tag_header[0] = type;
	ByteConverter::writeU24Forward(tag_header + 1, payload_size);
	tag_header[4] = (timestamp >> 16) & 0xFF;
	tag_header[5] = (timestamp >> 8)  & 0xFF;
	tag_header[6] = (timestamp >> 0)  & 0xFF;
	tag_header[7] = (timestamp >> 24) & 0xFF;

	ByteConverter::writeU32Forward(previous_tag_size, payload_size + 11);

	this->send(tag_header, 11);
	this->send(payload.get(), payload_size);
	this->send(previous_tag_size, 4);

	return 0;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
